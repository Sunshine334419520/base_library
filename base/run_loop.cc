/**
* @Author: YangGuang
* @Date:   2018-10-31
* @Email:  guang334419520@126.com
* @Filename: run_loop.cc
* @Last modified by:  YangGuang
*/

#include "base/run_loop.h"

#include <thread>

#include "base/callback.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/logging.h"

namespace base {

namespace {

thread_local LazyInstance<RunLoop::Delegate>::Leaky tls_delegate =
	LAZY_INSTANCE_INITIALIZER;

// 如果任务是运行在当前的序列上就直接运行他，否则的话就传递这个任务.
void ProxyToTaskRunner(std::shared_ptr<SequencedTaskRunner> task_runner,
					   Closure closure) {
	if (task_runner->RunsTasksInCurrentSequence()) {
		std::move(closure).operator();
		return;
	}
	task_runner->PostTask(FROM_HERE, std::move(closure));
}

}	// namespace.

RunLoop::Delegate::Delegate() {}

RunLoop::Delegate::~Delegate() {
	if (bound_) {
		tls_delegate.private_instance_.store(
			nullptr, std::memory_order_relaxed);
	}
}

bool RunLoop::Delegate::ShouldQuitWhenIdle() {
	return active_run_loops_.top()->quit_when_idle_received_;
}

// static 
void RunLoop::RegisterDelegateForCurrentThread(Delegate* delegate) {
	// 绑定 delegate 到当前的线程上.
	DCHECK(!delegate->bound_);

	// 可能在之前这个线程已经绑定过了，这样会报错.
	DCHECK(!tls_delegate.Pointer());
	tls_delegate.private_instance_.store(delegate,
										 std::memory_order_acq_rel);
	delegate->bound_ = true;
}

RunLoop::RunLoop(Type type)
	: delegate_(tls_delegate.Pointer()),
   	  type_(type),
	  origin_task_runner_(ThreadTaskRunnerHandle::Get()),
	  weak_factory_(std::shared_ptr<RunLoop>(std::move(this))) {
	DCHECK(delegate_);
	DCHECK(origin_task_runner_);
}

RunLoop::~RunLoop() {

}

void RunLoop::Run() {
	if (!BeforeRun())
		return;

	DCHECK_EQ(this, delegate_->active_run_loops_.top());
	const bool application_tasks_allowed =
		delegate_->active_run_loops_.size() == 1U ||
		type_ == Type::kNestableTasksAllowed;
	delegate_->Run(application_tasks_allowed);

	AfterRun();
}

void RunLoop::RunUntilIdle() {
	quit_when_idle_received_ = true;
	Run();
}

void RunLoop::Quit() {
	// Thread-safe.
	// 如果在绑定的线程上，就运行delegate_->Quit(), 否则就把这个Quit发送到
	// origin_task_runner_绑定的线程的消息队列上.
	if (!origin_task_runner_->RunsTasksInCurrentSequence()) {
		origin_task_runner_->PostTask(
			FROM_HERE, std::bind(&RunLoop::Quit, this));
		return;
	}

	quit_called_ = true;
	if (running_ && delegate_->active_run_loops_.top() == this) {
		delegate_->Quit();
	}
}

void RunLoop::QuitWhenIdle() {
	// Thread-safe
	// 如果在绑定的线程上，就等到线程事闲置的时候退出, 否则就把这个Quit发送到
	// origin_task_runner_绑定的线程的消息队列上.
	if (!origin_task_runner_->RunsTasksInCurrentSequence()) {
		origin_task_runner_->PostTask(
			FROM_HERE, std::bind(&RunLoop::QuitWhenIdle, this));
		return;
	}
	
	quit_when_idle_received_ = true;
}

base::Closure RunLoop::QuitClosure() {
	allow_quit_current_deprecated_ = false;

	return std::bind(&ProxyToTaskRunner, origin_task_runner_,
					 std::bind(&RunLoop::Quit, weak_factory_));
}

base::Closure RunLoop::QuitWhenIdleClosure() {
	allow_quit_current_deprecated_ = false;

	return std::bind(&ProxyToTaskRunner, origin_task_runner_,
					 std::bind(&RunLoop::QuitWhenIdle, weak_factory_));
}

// static.
bool RunLoop::IsRunningOnCurrentThread() {
	Delegate* delegate = tls_delegate.Pointer();
	return delegate && !delegate->active_run_loops_.empty();
}

// static.
bool RunLoop::IsNestedOnCurrentThread() {
	Delegate* delegate = tls_delegate.Pointer();
	return delegate && delegate->active_run_loops_.size() > 1;
}

// static.
void RunLoop::AddNestingObserverOnCurrentThread(NestingObserver* observer) {
	Delegate* delegate = tls_delegate.Pointer();
	DCHECK(delegate);
	delegate->nesting_observers_.push_back(
		std::shared_ptr<NestingObserver>(std::move(observer)));
}





}