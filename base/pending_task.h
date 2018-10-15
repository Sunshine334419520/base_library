/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: callback.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_PENDING_TASK_H
#define BASE_PENDING_TASK_H

#include <array>
#include <queue>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"


namespace base {

enum class Nestable {
	kNonNestable,
	kNestable,
};

struct BASE_EXPORT PendingTask {

	Closure task;
	
	Location posted_from;

	std::array<const void*, 4> task_backtrace;

	int sequence_num;

	Nestable nestable;

	bool is_high_res;
};

using TaskQueue = std::queue<PendingTask>;

using DelayedTaskQueue = std::priority_queue<PendingTask>;

}		// namespace base.

#endif // !BASE_PENDING_TASK_H
