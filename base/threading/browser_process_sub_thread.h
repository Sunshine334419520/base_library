/**
* @Author: YangGuang
* @Date:   2018-11-22
* @Email:  guang334419520@126.com
* @Filename: browser_process_sub_thread.h
* @Last modified by:  YangGuang
*/

#ifndef SUN_BROWSER_PROCESS_SUB_THREAD_H
#define SUN_BROWSER_PROCESS_SUB_THREAD_H

#include <memory>

#include "base/macor.h"
#include "base/threading/thread.h"
#include "base/threading/browser_thread.h"

namespace sun {

class BrowserProcessSubThread : public base::Thread {
 public:
	 explicit BrowserProcessSubThread(BrowserThread::ID identifier);
	 ~BrowserProcessSubThread() override;

	 // 将当前的线程的 |identifier_|注册到BrowserThread，这个方法必须调用在
	 // 线程已经处于运行的状态,而且这个方法只能调用一次.
	 void RegisterAsBrowserThread();

	 // 创建并且开始这个IO thread.
	 static std::unique_ptr<BrowserProcessSubThread> CreateIOThread();

 protected:
	 void Init() override;
	 void Run(base::RunLoop* run_loop) override;
	 void CleanUp() override;

 private:
	 // 第二个初始化阶段，这个只应该在RegisterAsBrowserThread()上调用.
	 void CompleteInitializationOnBrowserThread();

	 void UIThreadRun(base::RunLoop* run_loop);
	 void IOThreadRun(base::RunLoop* run_loop);

	 void IOThreadCleanUp();

	 const BrowserThread::ID identifier_;

	 std::unique_ptr<BrowserThreadImpl> browser_thread_;

	 DISALLOW_COPY_AND_ASSIGN(BrowserProcessSubThread);
};

}

#endif