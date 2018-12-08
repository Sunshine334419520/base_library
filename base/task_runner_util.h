/**
* @Author: YangGuang
* @Date:   2018-11-13
* @Email:  guang334419520@126.com
* @Filename: task_runner.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_TASK_RUNNER_UTIL_H
#define BASE_TASK_RUNNER_UTIL_H

#include <utility>

#include "base/callback.h"
#include "base/logging.h"
#include "base/bind_util.h"
#include "base/post_task_and_reply_with_result_internal.h"
#include "base/task_runner.h"

namespace base {

// 现在假设你有了一下的函数
// R DoWorkAndReturn();
// void Callback(const R& result);
//
// 这个PostTaskAndReplyWithResult可以这样来使用 for example:
// PostTaskAndReplyWithResult(
//	target_thread_.task_runner(),
//  FROM_HERE,
//  std::bind(&DoWorkAndReturn),
//	std::bind(&Callback));
template <typename TaskReturnType, typename ReplyArgType>
bool PostTaskAndReplyWithResult(TaskRunner* task_runner,
								const Location& from_here,
								base::Callback<TaskReturnType()> task,
								base::Callback<void(ReplyArgType)> reply) {
	DCHECK_NOTNULL(task);
	DCHECK_NOTNULL(reply);
	printf("PostTaskAndReplyWithResult\n");
	TaskReturnType* result = new TaskReturnType();
	return task_runner->PostTaskAndReplay(
		from_here,
		base::BindOnceClosure(&internal::ReturnAsParamAdapter<TaskReturnType>,
		std::move(task), result),
		base::BindOnceClosure(&internal::ReplayAdapter<TaskReturnType, ReplyArgType>,
		std::move(reply), result));
	return false;
}

}	// namespace base.

#endif // !BASE_TASK_RUNNER_UTIL_H