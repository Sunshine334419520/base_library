/**
* @Author: YangGuang
* @Date:   2018-10-20
* @Email:  guang334419520@126.com
* @Filename: task_runner.h
* @Last modified by:  YangGuang
*/
#include "base/task_runner.h"

#include <utility>

#include "base/logging.h"
#include "base/threading/post_task_and_reply_impl.h"

namespace base {

namespace {

class PostTaskAndReplyTaskRunner : public internal::PostTaskAndReplayImpl {
 public:
	 explicit PostTaskAndReplyTaskRunner(TaskRunner* destination);
 private:
	 bool PostTask(const Location& from_here, Closure task) OVERRIDE;

	 TaskRunner* destination_;
};

PostTaskAndReplyTaskRunner::PostTaskAndReplyTaskRunner(
	TaskRunner* destination) : destination_(destination) {
	DCHECK_NOTNULL(destination_);
}

bool PostTaskAndReplyTaskRunner::PostTask(const Location& from_here,
										  Closure task) {
	return destination_->PostTask(from_here, task);
}

}	// namespace .


bool TaskRunner::PostTask(const Location & from_here, Closure task) {
	return PostDelayedTask(from_here, std::move(task),
						   std::chrono::milliseconds(0));
}

bool TaskRunner::PostTaskAndReplay(const Location & from_here,
								   Closure task,
								   Closure reply) {
	return PostTaskAndReplyTaskRunner(this).PostTaskAndReply(
		from_here, task, reply);
}

TaskRunner::TaskRunner() = default;

TaskRunner::~TaskRunner() = default;

void TaskRunner::OnDestruct() const {
	delete this;
}

void TaskRunnerTraits::Destruct(const TaskRunner * task_runner) {
	task_runner->OnDestruct();
}

}	// namepsace base.