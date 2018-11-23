/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: imcoming_task_queue.cc
* @Last modified by:  YangGuang
*/

#include "base/message_loop/incoming_task_queue.h"
#include "base/message_loop/message_loop.h"
#include "base/logging.h"

namespace base {

namespace internal {

namespace {

std::chrono::milliseconds 
CalculateDelayedRuntime(std::chrono::milliseconds delay) {
	std::chrono::milliseconds delayed_run_time(0);
	if (delay > std::chrono::milliseconds(0))
		delayed_run_time = std::chrono::duration_cast<
		std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() +
								   delay);
	else
		DCHECK_EQ(delay.count(), 0);

	return delayed_run_time;
}

}	// namespace .

IncomingTaskQueue::IncomingTaskQueue(MessageLoop * message_loop)
	: always_schedule_work_(false),
	  triage_tasks_(this),
	  delayed_tasks_(this),
	  deferred_tasks_(this),
	  message_loop_(message_loop) {
}

bool IncomingTaskQueue::AddToIncomingQueue(const Location & from_here,
										   OnceClosure task,
										   std::chrono::milliseconds delay,
										   Nestable nestable) {
	CHECK(!task.is_null());

	PendingTask pending_task(from_here, std::move(task),
							 CalculateDelayedRuntime(delay), nestable);

	return PostPendingTask(&pending_task);
}

bool IncomingTaskQueue::IsIdleForTesting() {
	std::lock_guard<std::mutex> lock(incoming_queue_lock_);
	return incoming_queue_.empty();
}

void IncomingTaskQueue::WillDestroyCurrentMessageLoop() {
	{
		std::lock_guard<std::mutex> lock(incoming_queue_lock_);
		accept_new_tasks_ = false;
	}
	{
		std::lock_guard<std::mutex> lock(message_loop_lock_);
		message_loop_ = nullptr;
	}
}

void IncomingTaskQueue::StartScheduling() {
	bool schedule_work;
	{
		std::lock_guard<std::mutex> lock(incoming_queue_lock_);
		DCHECK(!is_ready_for_schedulig_);
		DCHECK(!message_loop_scheduled_);
		is_ready_for_schedulig_ = true;
		schedule_work = !incoming_queue_.empty();
	}
	if (schedule_work) {
		DCHECK_NOTNULL(message_loop_);

		// 这里不需要加锁，因为这个只会在自己的线程上调用.
		message_loop_->SchedueWork();
	}
}

void IncomingTaskQueue::RunTask(PendingTask * pending_task) {
	// 运行任务.
	std::move(pending_task->task).Run();
}

IncomingTaskQueue::~IncomingTaskQueue() {
	// 用来验证WillDestroyCurrentMessageLoop()时调用了.
	DCHECK(!message_loop_);
}

bool IncomingTaskQueue::PostPendingTask(PendingTask * pending_task) {
	bool accept_new_tasks;
	bool schedule_work = false;

	{
		std::lock_guard<std::mutex> lock(incoming_queue_lock_);
		accept_new_tasks = accept_new_tasks_;
		if (accept_new_tasks) {
			schedule_work =
				PostPendingTaskLockRequired(pending_task);
		}
	}

	if (!accept_new_tasks) {
		DCHECK(!schedule_work);
		pending_task->task.Reset();
		return false;
	}

	// 唤醒message loop 并且给他派遣工作
	if (schedule_work) {
		// 锁住message loop, 防止message loop被释放.
		std::lock_guard<std::mutex> lock(message_loop_lock_);
		if (message_loop_)
			message_loop_->SchedueWork();
	}

	return true;
}

bool IncomingTaskQueue::PostPendingTaskLockRequired(PendingTask * pending_task) {
	//std::lock_guard<std::mutex> lock(incoming_queue_lock_);

	pending_task->sequence_num = next_sequence_num_++;

	bool was_empty = incoming_queue_.empty();
	incoming_queue_.push(std::move(*pending_task));

	// 当is_ready_for_schedulig_为true时，代表是已经调用了StartScheulig, 
	// 如果always_schedule_work_为true，表示可以一直派遣工作，并且唤醒message loop
	// 那样可以返回true，如果是第一个派遣工作也可以直接返回true.
	if (is_ready_for_schedulig_ &&
		(always_schedule_work_ || (!message_loop_scheduled_ && was_empty))) {
		message_loop_scheduled_ = true;
		return true;
	}

	return false;
}


int IncomingTaskQueue::ReloadWorkQueue(TaskQueue * work_queue) {
	// work queue 必须为空
	DCHECK(work_queue->empty());

	// 加锁
	std::lock_guard<std::mutex> lock(incoming_queue_lock_);
	if (incoming_queue_.empty()) {
		// 如果incoming queue为空的话，那么就代表这个incoming queue里面没有
		// 任何的任务，这种情况意味着将需要sleep然后等待任务到来，如果这个
		// incoming queue不是空的，那么就重新派遣任务, 这样就可以将
		// message_loop_scheduled_ 设置为false，让incoming queue不为空时，
		// 可以派遣任务.
		message_loop_scheduled_ = false;
	}
	else {
		incoming_queue_.swap(*work_queue);
	}

	int high_res_tasks = high_res_task_count_;
	high_res_task_count_ = 0;
	return high_res_tasks;
}

IncomingTaskQueue::TriageQueue::TriageQueue(IncomingTaskQueue * outer)
	: outer_(outer){
}

IncomingTaskQueue::TriageQueue::~TriageQueue() = default;

const PendingTask & IncomingTaskQueue::TriageQueue::Peek() {
	ReloadFromIncomingQueueIfEmpty();
	DCHECK(!queue_.empty());
	return queue_.front();
}

PendingTask IncomingTaskQueue::TriageQueue::Pop() {
	ReloadFromIncomingQueueIfEmpty();
	DCHECK(!queue_.empty());
	PendingTask pending_task = std::move(queue_.front());
	queue_.pop();

	if (pending_task.is_high_res)
		--outer_->pending_high_res_tasks_;
	
	return pending_task;
}

bool IncomingTaskQueue::TriageQueue::HasTasks() {
	ReloadFromIncomingQueueIfEmpty();
	return !queue_.empty();
}

void IncomingTaskQueue::TriageQueue::Clear() {
	while (!queue_.empty()) {
		PendingTask pending_task = std::move(queue_.front());
		queue_.pop();

		if (pending_task.is_high_res)
			--outer_->pending_high_res_tasks_;

		if (pending_task.delayed_run_time.count()) {
			outer_->delayed_tasks().Push(std::move(pending_task));
		}
	}
}

void IncomingTaskQueue::TriageQueue::ReloadFromIncomingQueueIfEmpty() {
	if (queue_.empty()) {
		outer_->pending_high_res_tasks_ += outer_->ReloadWorkQueue(&queue_);
	}
}


IncomingTaskQueue::DelayedQueue::DelayedQueue(IncomingTaskQueue * outer)
	: outer_(outer){
}

IncomingTaskQueue::DelayedQueue::~DelayedQueue() = default;

const PendingTask & IncomingTaskQueue::DelayedQueue::Peek() {
	DCHECK(!queue_.empty());
	return queue_.top();
}

PendingTask IncomingTaskQueue::DelayedQueue::Pop() {
	DCHECK(!queue_.empty());
	PendingTask delayed_task = std::move(const_cast<PendingTask&>(queue_.top()));
	queue_.pop();

	if (delayed_task.is_high_res)
		--outer_->pending_high_res_tasks_;
	return delayed_task;
}

bool IncomingTaskQueue::DelayedQueue::HasTasks() {
	//return !queue_.empty();
	while (!queue_.empty() && Peek().task.is_null())
		Pop();
	
	return !queue_.empty();
}

void IncomingTaskQueue::DelayedQueue::Clear() {
	while (!queue_.empty())
		Pop();
}

void IncomingTaskQueue::DelayedQueue::Push(PendingTask pending_task) {
	if (pending_task.is_high_res)
		++outer_->pending_high_res_tasks_;

	queue_.push(std::move(pending_task));
}

IncomingTaskQueue::DeferredQueue::DeferredQueue(IncomingTaskQueue * outer)
	: outer_(outer) {
}

IncomingTaskQueue::DeferredQueue::~DeferredQueue() = default;

const PendingTask & IncomingTaskQueue::DeferredQueue::Peek() {
	DCHECK(!queue_.empty());
	return queue_.front();
}

PendingTask IncomingTaskQueue::DeferredQueue::Pop() {
	DCHECK(!queue_.empty());
	PendingTask deferred_task = std::move(queue_.front());
	queue_.pop();

	if (deferred_task.is_high_res)
		--outer_->pending_high_res_tasks_;
	
	return deferred_task;
}

bool IncomingTaskQueue::DeferredQueue::HasTasks() {
	return !queue_.empty();
}

void IncomingTaskQueue::DeferredQueue::Clear() {
	while (!queue_.empty())
		Pop();
}

void IncomingTaskQueue::DeferredQueue::Push(PendingTask pending_task) {
	if (pending_task.is_high_res)
		++outer_->pending_high_res_tasks_;
	
	queue_.push(std::move(pending_task));
}

}	// namespace internal.

}	// namespace base.


