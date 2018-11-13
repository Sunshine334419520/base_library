/**
* @Author: YangGuang
* @Date:   2018-11-13
* @Email:  guang334419520@126.com
* @Filename: task_runner.h
* @Last modified by:  YangGuang
*/

#include <utility>

#include "base/callback.h"
#include "base/logging.h"
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
	DCHECK(task);
	DCHECK(reply);
	TaskReturnType* result = new TaskReturnType();
	// ...
	return true;
}
}