/**
* @Author: YangGuang
* @Date:   2018-10-24
* @Email:  guang334419520@126.com
* @Filename: sequenced_task_runner_handle.cc
* @Last modified by:  YangGuang
*/
#include "base/sequenced_task_runner_handle.h"

#include <utility>
#include <thread>

#include "base/lazy_instance.h"
#include "base/logging.h"

namespace base {

namespace {

// 一个线程局部的变量.
thread_local LazyInstance<SequencedTaskRunnerHandle>::Leaky
	sequenced_task_runner_tls = LAZY_INSTANCE_INITIALIZER;

}	// namespace.

std::shared_ptr<SequencedTaskRunner> 
SequencedTaskRunnerHandle::Get() {
	const SequencedTaskRunnerHandle* handle =
		sequenced_task_runner_tls.Pointer();

	if (handle)
		return handle->task_runner_;

	/*
	// Note if you hit this: the problem is the lack of a sequenced context. The
	// ThreadTaskRunnerHandle is just the last attempt at finding such a context.
	CHECK(ThreadTaskRunnerHandle::IsSet())
		<< "Error: This caller requires a sequenced context (i.e. the "
		"current task needs to run from a SequencedTaskRunner).";
	return ThreadTaskRunnerHandle::Get();
	*/
}

bool SequencedTaskRunnerHandle::IsSet() {
	return sequenced_task_runner_tls.Pointer() != nullptr;
	
	/*
	return sequenced_task_runner_tls.Pointer() != nullptr ||
		   ThreadTaskRunnerHandle::IsSet();
		   */
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
	std::shared_ptr<SequencedTaskRunner> task_runner)
	: task_runner_(std::move(task_runner)) {
	DCHECK(task_runner_->RunsTasksInCurrentSequence());
	DCHECK(!SequencedTaskRunnerHandle::IsSet());

	sequenced_task_runner_tls.private_instance_.store(
		nullptr, std::memory_order_relaxed);
}


SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
	DCHECK(task_runner_->RunsTasksInCurrentSequence());
	DCHECK_EQ(sequenced_task_runner_tls.Pointer(), this);

	sequenced_task_runner_tls.private_instance_.store(
		nullptr, std::memory_order_relaxed);
}


}	// namespace base