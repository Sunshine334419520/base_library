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
#include <chrono>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"

//using namespace std;

namespace base {

enum class Nestable {
	kNonNestable,
	kNestable,
};

struct BASE_EXPORT PendingTask {
	PendingTask(const Location& posted_from,
				Closure task,
				std::chrono::milliseconds delayed_run_time = std::chrono::milliseconds(0),
				Nestable nestable = Nestable::kNestable);
	PendingTask(PendingTask&& other);
	~PendingTask();

	PendingTask& operator=(PendingTask&& other);

	// Used to support sorting.
	bool operator<(const PendingTask& other) const;

	Closure task;
	
	Location posted_from;

	std::chrono::milliseconds delayed_run_time;

	std::array<const void*, 4> task_backtrace;

	int sequence_num;

	Nestable nestable;

	bool is_high_res;
};

using TaskQueue = std::queue<PendingTask>;

using DelayedTaskQueue = std::priority_queue<PendingTask>;

}		// namespace base.

#endif // !BASE_PENDING_TASK_H
