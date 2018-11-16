/**
* @Author: YangGuang
* @Date:   2018-10-21
* @Email:  guang334419520@126.com
* @Filename: post_task_and_reply_impl.h
* @Last modified by:  YangGuang
*/

// This file contains the implementation for TaskRunner::PostTaskAndReply.

#ifndef BASE_THREADING_POST_TASK_ADN_REPLY_IMPL_H
#define BASE_THREADING_POST_TASK_ADN_REPLY_IMPL_H

#include "base/base_export.h"
#include "base/macor.h"
#include "base/location.h"
#include "base/callback.h"

namespace base {

namespace internal {

class BASE_EXPORT PostTaskAndReplayImpl {
 public: 
     PostTaskAndReplayImpl() = default;
     virtual ~PostTaskAndReplayImpl() = default;

     // Posts |task| by calling PostTask(). On completion, posts |reply| to the
     // origin sequence. Can only be called when
     bool PostTaskAndReply(const Location& from_here,
                           OnceClosure task,
                           OnceClosure reply);
 private: 
     virtual bool PostTask(const Location& from_here, OnceClosure task) = 0;
};

}   // namespace internal.

}   // namespace base.
    
#endif // BASE_THREADING_POST_TASK_ADN_REPLY_IMPL_H