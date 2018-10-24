/**
* @Author: YangGuang
* @Date:   2018-10-24
* @Email:  guang334419520@126.com
* @Filename: sequenced_task_runner_handle.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H
#define BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H

#include <memory>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/sequenced_task_runner.h"

namespace base {

class BASE_EXPORT SequencedTaskRunnerHandle {
 public:
	 static std::shared_ptr<SequencedTaskRunner> Get();

	 // 如果返回true的话是满足下列的条件的:
	 // a) 一个SequencedTaskRunner是已经分配到当前的线程了，
	 //	   通过SequencedTaskRunnerHandle实例化.
	 // b) 当前线程有一个ThreadTaskRunnerHandle(包括任何
	 //	   有MessageLoop关联的线程).
	 static bool IsSet();

	 explicit SequencedTaskRunnerHandle(
		 std::shared_ptr<SequencedTaskRunner> task_runner);

	 SequencedTaskRunnerHandle() = default;
	 ~SequencedTaskRunnerHandle();

 private:
	 std::shared_ptr<SequencedTaskRunner> task_runner_;

	 DISALLOW_COPY_AND_ASSIGN(SequencedTaskRunnerHandle);
};

}	// namespace base.

#endif // !BASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H
