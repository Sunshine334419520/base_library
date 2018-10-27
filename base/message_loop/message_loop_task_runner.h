/**
* @Author: YangGuang
* @Date:   2018-10-25
* @Email:  guang334419520@126.com
* @Filename: message_loop_task_runner.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_MESSAGE_MESSAGE_LOOP_TASK_RUNNER_H
#define BASE_MESSAGE_MESSAGE_LOOP_TASK_RUNNER_H 

#include <memory>
#include <mutex>

#include "base/base_export.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/platform_thread.h"


namespace base {
namespace internal {

class IncomingTaskQueue;

class BASE_EXPORT MessageLoopTaskRunner : public SingleThreadTaskRunner {
 public:
	explicit MessageLoopTaskRunner(
		std::shared_ptr<IncomingTaskQueue> incoming_queue);

	// Initialize this message loop task runner on the current thread.
	void BindToCurrentThread();

	bool PostDelayedTask(const Location& from_here,
						 Closure Task,
						 std::chrono::milliseconds delay) OVERRIDE;

	bool PostNonNestableDelayedTask(const Location& from_here,
									Closure task,
									std::chrono::milliseconds delay) OVERRIDE;

	virtual bool RunsTasksInCurrentSequence() OVERRIDE;

 private:
	 ~MessageLoopTaskRunner() OVERRIDE;

	 std::shared_ptr<IncomingTaskQueue> incoming_queue_;

	 PlatformThreadId valid_thread_id_;
	 std::mutex valid_thread_id_lock_;
};

}	// namespace internal.

}	// namespace base.


#endif // !BASE_MESSAGE_MESSAGE_LOOP_TASK_RUNNER_H
