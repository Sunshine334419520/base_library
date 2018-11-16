/**
* @Author: YangGuang
* @Date:   2018-10-26
* @Email:  guang334419520@126.com
* @Filename: message_loop_task_runner.cc
* @Last modified by:  YangGuang
*/
#include "base/message_loop/message_loop_task_runner.h"

#include <utility>

#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/incoming_task_queue.h"

namespace base {

namespace internal {


MessageLoopTaskRunner::MessageLoopTaskRunner(
	std::shared_ptr<IncomingTaskQueue> incoming_queue) 
	: incoming_queue_(incoming_queue){
}

void MessageLoopTaskRunner::BindToCurrentThread() {
	std::lock_guard<std::mutex> lock(valid_thread_id_lock_);
	DCHECK_EQ(kInvalidThreadId, valid_thread_id_);
	valid_thread_id_ = PlatformThread::CurrentId();
}

bool MessageLoopTaskRunner::PostDelayedTask(const Location& from_here,
											OnceClosure task,
											std::chrono::milliseconds delay) {
	DCHECK(!task.is_null());

	return incoming_queue_->AddToIncomingQueue(from_here, std::move(task), delay,
											   Nestable::kNestable);
}

bool MessageLoopTaskRunner::PostNonNestableDelayedTask(const Location& from_here,
													   OnceClosure task,
													   std::chrono::milliseconds delay) {
	DCHECK(!task.is_null());

	return incoming_queue_->AddToIncomingQueue(from_here, std::move(task), delay,
											   Nestable::kNonNestable);
}

bool MessageLoopTaskRunner::RunsTasksInCurrentSequence() {
	std::lock_guard<std::mutex> lock(valid_thread_id_lock_);
	return valid_thread_id_ == PlatformThread::CurrentId();
}

MessageLoopTaskRunner::~MessageLoopTaskRunner() = default;

}	// namespace base.

}	// namespace internal.