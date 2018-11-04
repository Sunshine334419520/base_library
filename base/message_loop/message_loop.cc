/**
* @Author: YangGuang
* @Date:   2018-10-26
* @Email:  guang334419520@126.com
* @Filename: message_loop.cc
* @Last modified by:  YangGuang
*/
#include "base/message_loop/message_loop.h"

#include <utility>
#include <thread>

#include "base/message_loop/message_pump_default.h"
#include "base/logging.h"
#include "base/ptr_util.h"

namespace base {

namespace {


MessageLoop* GetTLSMessageLoop() {
	static thread_local MessageLoop* lazy_tls_ptr = new  MessageLoop();
	return lazy_tls_ptr;
}

MessageLoop::MessagePumpFactory* messge_pump_for_ui_factory_ = NULL;

std::unique_ptr<MessagePump> ReturnPump(MessagePump* pump) {
	return WrapUnique(std::move(pump));
}

}	// namespace .

MessageLoop::MessageLoop(Type type)
	: MessageLoop(type, MessagePumpFactoryCallback()) {
	BindToCurrentThread();
}

MessageLoop::MessageLoop(std::unique_ptr<MessagePump> pump)
	: MessageLoop(TYPE_CUSTOM, std::bind(&ReturnPump, pump.release())) {
	BindToCurrentThread();

	//pump_factory_ = std::bind(&ReturnPump, pump);
}

MessageLoop::~MessageLoop()
{

}

// static.
MessageLoop * MessageLoop::current() {
	return GetTLSMessageLoop();
}



std::unique_ptr<MessagePump> 
MessageLoop::CreateMessagePumpForType(Type type) {
	if (type == MessageLoop::TYPE_UI) {

	}

	if (type == MessageLoop::TYPE_IO) {

	}

	DCHECK_EQ(type, MessageLoop::TYPE_DEFAULT);
	return std::make_unique<MessagePumpDefault>();
}

void MessageLoop::AddDestructionObserver(
	std::shared_ptr<DestructionObserver> destruction_observer) {
	DCHECK_EQ(this, current());
	destruction_observers_.push_back(std::move(destruction_observer));
}

void MessageLoop::RemoveDestructionObserver(
	std::shared_ptr<DestructionObserver> destruction_observer) {
	DCHECK_EQ(this, current());
	destruction_observers_.remove(std::move(destruction_observer));
}

Closure MessageLoop::QuitWhenIdleClosure() {
	//return std::bind(&RunLoop::QuitCurrentWhenIdleDeprecated);
	return Closure();
}

bool MessageLoop::IsType(Type type) const {
	return type_ == type;
}

std::string MessageLoop::GetThreadName() const
{
	return std::string();
}

void MessageLoop::ClearTaskRunnerForTesting()
{
}

void MessageLoop::SetNestableTasksAllowed(bool allowed) {
	if (allowed) {
		//CHECK(RunLoop::IsNestingAllowedOnCurrentThread());

		// Kick the native pump just in case we enter a OS-driven nested message
		// loop that does not go through RunLoop::Run().
		pump_->ScheduleWork();
	}
	task_execution_allowed_ = allowed;
}

void MessageLoop::Run(bool application_tasks_allowed) {
	if (application_tasks_allowed && !task_execution_allowed_) {
		task_execution_allowed_ = true;
		pump_->Run(this);
		task_execution_allowed_ = false;
	}
	else {
		pump_->Run(this);
	}
}

void MessageLoop::Quit() {
	pump_->Quit();
}

void MessageLoop::EnsureWorkScheduled() {
	if (incoming_task_queue_->triage_tasks().HasTasks())
    	pump_->ScheduleWork();
}

bool MessageLoop::NestableTasksAllowed() const {
	return task_execution_allowed_;
}

void MessageLoop::AddTaskObserver(
	std::shared_ptr<TaskObserver> task_observer) {
	DCHECK_EQ(this, current());
	CHECK(allow_task_observers_);
	task_observers_.push_back(std::move(task_observer));
}

void MessageLoop::RemoveTaskObserver(
	std::shared_ptr<TaskObserver> task_observer) {
	DCHECK_EQ(this, current());
	CHECK(allow_task_observers_);
	task_observers_.push_back(std::move(task_observer));
}

bool MessageLoop::IsIdleForTesting()
{
	return false;
}




MessageLoop::MessageLoop(Type type, 
						 MessagePumpFactoryCallback pump_factory)
	: type_(type),
	  pump_factory_(std::move(pump_factory)),
	  incoming_task_queue_(WrapShared(new internal::IncomingTaskQueue(this))),
	  unbound_task_runner_(
		WrapShared(new internal::MessageLoopTaskRunner(incoming_task_queue_))),
	  task_runner_(unbound_task_runner_){
	// 如果类型是TYPE_CUSTOM 那么pump_factory 必须不为空.
	DCHECK(type_ != TYPE_CUSTOM || !pump_factory_);
}

void MessageLoop::BindToCurrentThread() {
	DCHECK(!pump_);
	if (!pump_factory_)
		pump_ = std::move(pump_factory_)();
	else
		pump_ = CreateMessagePumpForType(type_);

	// 设置当前为this
	DCHECK(!current());
	auto current_point = GetTLSMessageLoop();
	current_point = this;

	incoming_task_queue_->StartScheduling();
	unbound_task_runner_->BindToCurrentThread();
	unbound_task_runner_ = nullptr;
	SetThreadTaskRunnerHandle();
	thread_id_ = PlatformThread::CurrentId();
	
	RunLoop::RegisterDelegateForCurrentThread(this);
}

std::unique_ptr<MessageLoop>
MessageLoop::CreateUnbound(Type type,
						   MessagePumpFactoryCallback pump_factory) {
	return std::unique_ptr<MessageLoop>(new MessageLoop(type,
										std::move(pump_factory)));
}

void MessageLoop::SetThreadTaskRunnerHandle() {
	thread_task_runner_handle_.reset();
	thread_task_runner_handle_.reset(new ThreadTaskRunnerHandle(task_runner_));
}

bool MessageLoop::ProcessNextDelayedNoNestableTask()
{
	return false;
}

void MessageLoop::RunTask(PendingTask * pending_task) {
	DCHECK(task_execution_allowed_);
	current_pending_task_ = pending_task;

	task_execution_allowed_ = false;

	for (auto& observer : task_observers_)
		observer->OnBeforeProcessTask(*pending_task);
	incoming_task_queue_->RunTask(pending_task);
	for (auto& observer : task_observers_)
		observer->OnAfterProcessTask(*pending_task);

	// 设置为true.
	task_execution_allowed_ = true;

	current_pending_task_ = nullptr;
}


bool MessageLoop::DeferOrRunPendingTask(PendingTask pending_task) {
	// 添加到闲置任务，或者直接运行任务.
	if (pending_task.nestable == Nestable::kNestable ||
		!RunLoop::IsNestedOnCurrentThread()) {
		RunTask(&pending_task);
		return true;
	}
	

	// 我们现在不能运行任务，因为我们现在处于一个nested run loop中,
	// 而且我们的任务还是一个no nestable, 所以我们不能运行这个任务，
	// 我们将他加入到deferred task中.
	incoming_task_queue_->deferred_tasks().Push(std::move(pending_task));
	return false;
}

void MessageLoop::DeletePendingTasks() {
	incoming_task_queue_->triage_tasks().Clear();
	incoming_task_queue_->deferred_tasks().Clear();
	incoming_task_queue_->delayed_tasks().Clear();
}

void MessageLoop::SchedueWork() {
	pump_->ScheduleWork();
}

bool MessageLoop::DoWork() {
	if (!task_execution_allowed_)
		return false;

	// Execute oldest task.
	while (incoming_task_queue_->triage_tasks().HasTasks()) {
		PendingTask pending_task = incoming_task_queue_->triage_tasks().Pop();
		if (!pending_task.task)
			continue;

		if (pending_task.delayed_run_time.count() != 0) {
			// 延迟时间不为0，是一个延迟任务，将他加入到延迟队列.
			int sequence_num = pending_task.sequence_num;
			auto delayed_run_time = pending_task.delayed_run_time;
			incoming_task_queue_->delayed_tasks().Push(std::move(pending_task));
			// 如果我Push进延迟队列的任务是最top的任务（换一句话说就是即将需要执行的任务),
			// 那么我们就需要重新设置一下延迟时间.
			if (incoming_task_queue_->delayed_tasks().Peek().sequence_num ==
				sequence_num) {
				// 刷新延迟时间.
				pump_->ScheduleDelayedWork(delayed_run_time);
			}
		}
		else if (DeferOrRunPendingTask(std::move(pending_task))) {
			return true;
		}
	}

	// 什么都没有发生.
	return false;
}

bool MessageLoop::DoDelayedWork(
	std::chrono::milliseconds& next_delayed_work_time) {
	if (!task_execution_allowed_ ||
		!incoming_task_queue_->delayed_tasks().HasTasks()) {
		// 没有任务，或者是不允许执行任务时，我们更新最新的时间.
		recent_time = next_delayed_work_time = 
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
		return false;
	}

	// 当我们“落后”时，延迟的工作队列中会有很多任务准备好运行。为了提高效率，
	// 当我们落后时，我们将只间歇地调用Now()，然后在再次调用之前处理
	// 所有准备运行的任务。因此，我们落后的越多(并且有许多准备运行的延迟任务)，
	// 我们处理这些任务的效率就越高。
	auto next_run_time =
		incoming_task_queue_->delayed_tasks().Peek().delayed_run_time;
	if (next_run_time > recent_time) {
		// 如果延迟时间大于我们最新的时间,我们重新获取最新的时间，并且重新比较.
		recent_time = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
		if (next_run_time > recent_time) {
			// 还是大于需要延迟的时间，返回并且等待到达延迟时间再执行.
			next_delayed_work_time = next_run_time;
			return false;
		}
	}

	// 延迟时间已到，延迟任务可以执行了
	PendingTask pending_task = incoming_task_queue_->delayed_tasks().Pop();

	if (incoming_task_queue_->delayed_tasks().HasTasks()) {
		next_delayed_work_time =
			incoming_task_queue_->delayed_tasks().Peek().delayed_run_time;
	}

	return DeferOrRunPendingTask(std::move(pending_task));
}

bool MessageLoop::DoIdleWork() {
	if (ProcessNextDelayedNoNestableTask())
		return true;

	
	if (ShouldQuitWhenIdle())
		pump_->Quit();
		

	return false;
}

MessageLoop::DestructionObserver::~DestructionObserver()
{
}

MessageLoop::TaskObserver::TaskObserver()
{
}

}