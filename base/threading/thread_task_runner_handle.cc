/**
* @Author: YangGuang
* @Date:   2018-10-25
* @Email:  guang334419520@126.com
* @Filename: thread_task_runner_handle.cc
* @Last modified by:  YangGuang
*/

#include "base/threading/thread_task_runner_handle.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/sequenced_task_runner_handle.h"



namespace base {

namespace {

thread_local LazyInstance<ThreadTaskRunnerHandle*>::Leaky 
	thread_task_runner_tls = LAZY_INSTANCE_INITIALIZER;

}

std::shared_ptr<SingleThreadTaskRunner> 
ThreadTaskRunnerHandle::Get() {
	ThreadTaskRunnerHandle* current = thread_task_runner_tls.Get();

	DCHECK(current);
	return current->task_runner_;
}

bool ThreadTaskRunnerHandle::IsSet() {
	return !!(*thread_task_runner_tls.Pointer());
}

ThreadTaskRunnerHandle::ThreadTaskRunnerHandle(
	std::shared_ptr<SingleThreadTaskRunner> task_runner) 
	: task_runner_(task_runner) {
	
	DCHECK(task_runner_->BelongsToCurrentThread());

	DCHECK(!SequencedTaskRunnerHandle::IsSet());

	/*auto temp = thread_task_runner_tls.Get();
	temp = this;*/
	*thread_task_runner_tls.private_instance_ = this;

	auto a = thread_task_runner_tls.Get();

	DCHECK(SequencedTaskRunnerHandle::IsSet());

	/*
	thread_task_runner_tls.private_instance_.store(&tmp,
												   std::memory_order_relaxed);
												   */
}

ThreadTaskRunnerHandle::~ThreadTaskRunnerHandle() {
	DCHECK(task_runner_->BelongsToCurrentThread());
	DCHECK_EQ(thread_task_runner_tls.Get(), this);

	thread_task_runner_tls.private_instance_.store(nullptr, 
												   std::memory_order_relaxed);
}

}	// namespace base.