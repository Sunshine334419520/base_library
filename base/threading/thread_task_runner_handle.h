/**
* @Author: YangGuang
* @Date:   2018-10-25
* @Email:  guang334419520@126.com
* @Filename: thread_task_runner_handle.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H
#define BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H 

#include <memory>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/single_thread_task_runner.h"

namespace base {

class BASE_EXPORT ThreadTaskRunnerHandle {
 public:
	 static std::shared_ptr<SingleThreadTaskRunner> Get();

	 // 如果返回true的话是满足下列的条件的:
	 // a) 一个SingleThreadTaskRunner是已经分配到当前的线程了，
	 //	   通过SingleThreadTaskRunner实例化.
	 // b) 当前线程有一个ThreadTaskRunnerHandle(包括任何
	 //	   有MessageLoop关联的线程).
	 static bool IsSet();

	 explicit ThreadTaskRunnerHandle(
		 std::shared_ptr<SingleThreadTaskRunner> task_runner);

	 ThreadTaskRunnerHandle() = default;
	 ~ThreadTaskRunnerHandle();

private:
	std::shared_ptr<SingleThreadTaskRunner> task_runner_;

	DISALLOW_COPY_AND_ASSIGN(ThreadTaskRunnerHandle);
};

}

#endif // !BASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H
