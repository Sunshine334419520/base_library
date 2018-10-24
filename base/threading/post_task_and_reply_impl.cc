/**
* @Author: YangGuang
* @Date:   2018-10-21
* @Email:  guang334419520@126.com
* @Filename: post_task_and_reply_impl.cc
* @Last modified by:  YangGuang
*/
#include "base/threading/post_task_and_reply_impl.h"


#include "base/location.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "base/sequenced_task_runner_handle.h"


namespace base {

namespace {

class PostTaskAndReplyRelay {
 public:
	 PostTaskAndReplyRelay(const Location& from_here,
						   Closure task,
						   Closure reply)
		 : from_here_(from_here),
		   origin_task_runner_(SequencedTaskRunnerHandle::Get()),
		   reply_(std::move(reply)),
		   task_(std::move(task_)) {}

	 ~PostTaskAndReplyRelay() {

	 }

	 void RunTaskAndPostReply() {
		 std::move(task_)();

		 
		 origin_task_runner_->PostTask(from_here_,
									   std::bind(&PostTaskAndReplyRelay::RunReplyAndSelfDestruct,
									   this));
									   
	 }
 private:
	 void RunReplyAndSelfDestruct() {
		 
		 DCHECK_NULL(task_);

		 std::move(reply_)();

		 delete this;
	 }

	 const Location from_here_;
	 const std::shared_ptr<SequencedTaskRunner> origin_task_runner_;
	 Closure reply_;
	 Closure task_;

};

}	// namespace .

namespace internal {

bool PostTaskAndReplayImpl::PostTaskAndReply(const Location& from_here,
                                            Closure task,
                                            Closure reply) {
    DCHECK_NOTNULL(task);
    DCHECK_NOTNULL(reply);

	/*
    return PostTask(from_here,
                    std::bind(&PostTaskAndReplyRelay::RunTaskAndPostReply,
                              PostTaskAndReplyRelay(from_here, std::move(task),
                                                    std::move(reply))));
													*/

	PostTaskAndReplyRelay* relay =
		new PostTaskAndReplyRelay(from_here, std::move(task), std::move(reply));

	if (!PostTask(from_here,
		std::bind(&PostTaskAndReplyRelay::RunTaskAndPostReply, relay))) {
		delete relay;
		return false;
	}

	return true;
}

}   // namespace internal

}   // namespace base
