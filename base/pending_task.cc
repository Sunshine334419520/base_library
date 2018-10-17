/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: pending_task.cc
* @Last modified by:  YangGuang
*/

#include "base/pending_task.h"

#include "base/message_loop/message_loop.h"

namespace base {
PendingTask::PendingTask(const Location & posted_from,
						 Closure task, 
						 std::chrono::milliseconds delayed_run_time,
						 Nestable nestable)
	: task(std::move(task)),
	  posted_from(posted_from),
	  delayed_run_time(delayed_run_time),
	  sequence_num(0),
	  nestable(nestable),
	  is_high_res(false) {
	const PendingTask* parent_task =
		MessageLoop::current() ? MessageLoop::current()->current_pending_task_
		: nullptr;
	if (parent_task) {
		task_backtrace[0] = parent_task->posted_from.program_counter();
		std::copy(parent_task->task_backtrace.begin(),
				  parent_task->task_backtrace.end() - 1,
				  task_backtrace.begin() + 1);
	}
	else {
		task_backtrace.fill(nullptr);
	}
}

PendingTask::PendingTask(PendingTask && other) = default;

PendingTask::~PendingTask() = default;

PendingTask & PendingTask::operator=(PendingTask && other) = default;

bool PendingTask::operator<(const PendingTask & other) const
{
	if (delayed_run_time < other.delayed_run_time)
		return false;

	if (delayed_run_time > other.delayed_run_time)
		return true;

	// If the times happen to match, then we use the sequence number to decide.
	// Compare the difference to support integer roll-over.
	return (sequence_num - other.sequence_num) > 0;
}



}	// namespace base.