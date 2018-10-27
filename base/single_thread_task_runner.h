/**
* @Author: YangGuang
* @Date:   2018-10-24
* @Email:  guang334419520@126.com
* @Filename: single_thread_task_runner.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_SINGLE_THREAD_TASK_RUNNER_H
#define BASE_SINGLE_THREAD_TASK_RUNNER_H

#include "base/base_export.h"
#include "base/macor.h"
#include "base/sequenced_task_runner.h"

namespace base {

class BASE_EXPORT SingleThreadTaskRunner : public SequencedTaskRunner {
 public:
	 bool BelongsToCurrentThread() { return RunsTasksInCurrentSequence(); }

 protected:
	 ~SingleThreadTaskRunner() OVERRIDE = default;
};

}	// namespace base.

#endif // !BASE_SINGLE_THREAD_TASK_RUNNER_H

