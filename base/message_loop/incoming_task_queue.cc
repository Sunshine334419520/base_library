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
	std::chrono::milliseconds delayed_run_time;
	if (delay > std::chrono::milliseconds(0))
		delayed_run_time = std::chrono::duration_cast<
		std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() +
								   delay);
	else
		DCHECK_EQ(delay.count, 0);

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
										   Closure task,
										   std::chrono::milliseconds delay,
										   Nestable nestable) {
	CHECK_NOTNULL(task);

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
	std::move(pending_task->task)();
}

IncomingTaskQueue::~IncomingTaskQueue() {
	// 用来验证WillDestroyCurrentMessageLoop()时调用了.
	DCHECK(!message_loop_);
}

bool IncomingTaskQueue::PostPendingTask(PendingTask * pending_task)
{
	return false;
}

bool IncomingTaskQueue::PostPendingTaskLockRequired(PendingTask * pending_task)
{
	return false;
}

int IncomingTaskQueue::ReloadWorkQueue(TaskQueue * work_queue)
{
	return 0;
}

IncomingTaskQueue::TriageQueue::TriageQueue(IncomingTaskQueue * outer)
	: outer_(outer){
}

const PendingTask & IncomingTaskQueue::TriageQueue::Peek()
{
	// TODO: 在此处插入 return 语句
}

PendingTask IncomingTaskQueue::TriageQueue::Pop()
{
	return PendingTask();
}

bool IncomingTaskQueue::TriageQueue::HasTasks()
{
	return false;
}

void IncomingTaskQueue::TriageQueue::Clear()
{
}

void IncomingTaskQueue::TriageQueue::ReloadFromIncomingQueueIfEmpty()
{
}

IncomingTaskQueue::TriageQueue::~TriageQueue()
{
}

IncomingTaskQueue::DelayedQueue::DelayedQueue(IncomingTaskQueue * outer)
	: outer_(outer){
}

IncomingTaskQueue::DelayedQueue::~DelayedQueue()
{
}

const PendingTask & IncomingTaskQueue::DelayedQueue::Peek()
{
	// TODO: 在此处插入 return 语句
}

PendingTask IncomingTaskQueue::DelayedQueue::Pop()
{
	return PendingTask();
}

bool IncomingTaskQueue::DelayedQueue::HasTasks()
{
	return false;
}

void IncomingTaskQueue::DelayedQueue::Clear()
{
}

void IncomingTaskQueue::DelayedQueue::Push(PendingTask pending_task)
{
}

IncomingTaskQueue::DeferredQueue::DeferredQueue(IncomingTaskQueue * outer)
	: outer_(outer) {
}

IncomingTaskQueue::DeferredQueue::~DeferredQueue()
{
}

const PendingTask & IncomingTaskQueue::DeferredQueue::Peek()
{
	// TODO: 在此处插入 return 语句
}

PendingTask IncomingTaskQueue::DeferredQueue::Pop()
{
	return PendingTask();
}

bool IncomingTaskQueue::DeferredQueue::HasTasks()
{
	return false;
}

void IncomingTaskQueue::DeferredQueue::Clear()
{
}

void IncomingTaskQueue::DeferredQueue::Push(PendingTask pending_task)
{
}

}	// namespace internal.

}	// namespace base.


