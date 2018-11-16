/**
* @Author: YangGuang
* @Date:   2018-11-13
* @Email:  guang334419520@126.com
* @Filename: browser_thread.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_THREADING_THREAD_BROWSER_THREAD_H
#define BASE_THREADING_THREAD_BROWSER_THREAD_H

#include <memory>
#include <string>
#include <utility>
#include <chrono>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/macor.h"
#include "base/single_thread_task_runner.h"

namespace sun {

class BroserThreadImpl;

class BASE_EXPORT BrowserThread {
 public:
	 // 这个ID代表着你将要创建什么类型的线程，并且会对应的创建对应的message_loop.
	 enum ID {
		 UI,
		 IO,
		 IO_COUNT
	 };

	 // 这些函数都是一些与message_loop上的post task是一样的，他们会发送任务到
	 // 他们自己的线程的消息循环incoming queue, 如果消息循环还存在的话，任务会
	 // 发送成功，并且会返回true，否则失败，false，就算是发送成功了，也不能保证
	 // 这个任务一定会运行，因为又可能在之前发送过退出任务，那么这个退出任务将会
	 // 在它前面执行.
	 static bool PostTask(ID identifier,
						  const base::Location& from_here,
						  base::OnceClosure task);
	 static bool PostDelayedTask(ID identifier,
								 const base::Location& from_here,
								 base::OnceClosure task,
								 std::chrono::microseconds delay);
	 static bool PostNonNestableTask(ID identifier,
									 const base::Location& from_here,
									 base::OnceClosure task);
	 static bool PostNonNestableDelayedTask(ID identifier,
											const base::Location& from_here,
											base::OnceClosure task,
											std::chrono::milliseconds delay);

	 static bool PostTaskAndReply(ID identifier,
								  const base::Location& from_here,
								  base::OnceClosure task,
								  base::OnceClosure reply);

	 template <typename ReturnType, typename ReplyArgType>
	 static bool PostTaskAndReplyWithResult(
		 ID identifier,
		 const base::Location& from_here,
		 base::Callback<ReturnType()> task,
		 base::Callback<void(ReplyArgType)> reply) {
		 return false;
	 }

	 template <typename T>
	 static bool DeleteSoon(ID identifier,
							const base::Location& from_here,
							const T* object) {
		 return false;
	 }

	 template <typename T>
	 static bool DeleteSoon(ID identifier,
							const base::Location& from_here,
							std::unique_ptr<T> object) {
		 return DeleteSoon(identifier, from_here, object.release());
	 }

	 template <typename T>
	 static bool ReleaseSoon(ID identifier,
							 const base::Location& from_here,
							 const T* object) {
		 return false;
	 }

	 static void PostAfterStartupTask(
		 const base::Location& from_here,
		 const std::shared_ptr<base::TaskRunner>& task_runner,
		 base::OnceClosure task);

	 // 线程是否初始化，可以调用在任何线程上面.
	 static bool IsThreadinitialized(ID identifier) WARN_UNUSED_RESULT;

	 // 如果当前的线程与你给的|identifier|是同一个线程,返回true, 可以调用在任务线程上
	 static bool CurrentlyOn(ID identifier) WARN_UNUSED_RESULT;

	 // 如果当前消息循环是已知的线程中的一个，那么就返回true，并且设置ID到|identifier|
	 // 否则返回false.
	 static bool GetCurrentThreadIdentifier(ID* identifier) WARN_UNUSED_RESULT;

	 // 调用者可以在线程的生命周期之外持有一个被重新计算的任务运行器.
	 static std::shared_ptr<base::SingleThreadTaskRunner> GetTaskRunnerForThread(
		 ID identifier);

	 template <ID thread>
	 struct DeleteOnThread {
		 template <typename T>
		 static void Destruct(const T* x) {
			 if (CurrentlyOn(thread)) {
				 delete x;
			 }
			 else {
				 if (!DeleteSoon(thread, FROM_HERE, x)) {
					 LOG(logging::LogType::ERROR) << 
						 "DeleteSoon failed on thread" << thread;
				 }
			 }
		 }

		 template <typename T>
		 inline void operator()(T* ptr) const {
			 enum { type_must_be_complete = sizeof(T) };
			 Destruct(ptr);
		 }
	 };

 private:
	 friend class BrowserThreadImpl;
	 
	 BrowserThread() = default;
	 DISALLOW_COPY_AND_ASSIGN(BrowserThread);
};

}

#endif // !BASE_THREADING_THREAD_BROWSER_THREAD_H

