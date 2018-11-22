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
#include "base/bind_util.h"


namespace base {

namespace {

class PostTaskAndReplyRelay {
 public:
	 PostTaskAndReplyRelay(const Location& from_here,
						   OnceClosure task,
						   OnceClosure reply)
		 : from_here_(from_here),
		   origin_task_runner_(SequencedTaskRunnerHandle::Get()),
		   reply_(std::move(reply)),
		   task_(std::move(task_)) {}

	 ~PostTaskAndReplyRelay() {

	 }

	 void RunTaskAndPostReply() {
		 std::move(task_).Run();

		 
		 origin_task_runner_->PostTask(from_here_,
			BindOnceClosure(&PostTaskAndReplyRelay::RunReplyAndSelfDestruct,
							this));
									   
	 }
 private:
	 void RunReplyAndSelfDestruct() {
		 
		 DCHECK(task_.is_null()); 

		 std::move(reply_).Run();

		 delete this;
	 }

	 const Location from_here_;
	 const std::shared_ptr<SequencedTaskRunner> origin_task_runner_;
	 OnceClosure reply_;
	 OnceClosure task_;

};

}	// namespace .

namespace internal {

bool PostTaskAndReplayImpl::PostTaskAndReply(const Location& from_here,
                                             OnceClosure task,
                                             OnceClosure reply) {
	DCHECK(!task.is_null());
	DCHECK(!reply.is_null());

	/*
    return PostTask(from_here,
                    std::bind(&PostTaskAndReplyRelay::RunTaskAndPostReply,
                              PostTaskAndReplyRelay(from_here, std::move(task),
                                                    std::move(reply))));
													*/

	PostTaskAndReplyRelay* relay =
		new PostTaskAndReplyRelay(from_here, std::move(task), std::move(reply));

	if (!PostTask(from_here,
		BindOnceClosure(&PostTaskAndReplyRelay::RunTaskAndPostReply, relay))) {
		delete relay;
		return false;
	}

	return true;
}

}   // namespace internal

}   // namespace base
