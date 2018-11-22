/**
* @Author: YangGuang
* @Date:   2018-11-16
* @Email:  guang334419520@126.com
* @Filename: browser_thread.h
* @Last modified by:  YangGuang
*/
#ifndef SUN_BROWSER_THREAD_IMPL_H
#define SUN_BROWSER_THREAD_IMPL_H


#include "base/base_export.h"
#include "base/threading/browser_thread.h"

namespace sun {

class BrowserProcessSubThread;

// BrowserThreadImpl是一个作用域对象，它将一个SingleThreadTaskRunner
// 映射到BrowserThread::ID. 在这个~BrowserThreadImpl()上，将会将state
// 设置为SHUTDOWN(在这种情况下BrowserThread::IsThreadInitialized()将会
// 返回false)而且这个映射还没有取消(任务的runner是已经释放并且停止接受任何
// 的任务)
class BASE_EXPORT BrowserThreadImpl : public BrowserThread {
 public:
	 ~BrowserThreadImpl();

	 // 通过identifier获得线程的name.
	 static const char* GetThreadName(BrowserThread::ID identifier);

 private:
	 friend class BrowserProcessSubThread;

	 // 绑定这个|identifier| 到 |task_runner|.
	 BrowserThreadImpl(BrowserThread::ID identifier,
					   std::shared_ptr<base::SingleThreadTaskRunner> task_runner);

	 // thraed的id.
	 ID identifier_;
};

}	// namespace sun.



#endif // !SUN_BROWSER_THREAD_IMPL_H
