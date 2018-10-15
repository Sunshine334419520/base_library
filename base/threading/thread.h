/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: thread.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_THREADING_THREAD_H
#define BASE_THREADING_THREAD_H

#include <stddef.h>
#include <memory>
#include <string>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macor.h"
#include "base/threading/platform_thread.h"

//std::packaged_task<std::function<void()>>

namespace base {

class MessagePump;
class RunLoop;
class MessageLoop;

class BASE_EXPORT Thread : PlatformThread::Delegate {
 public:
	 struct BASE_EXPORT Options {
		 typedef std::function<std::unique_ptr<
			 MessagePump>()> MessagePumpFactory;

		 Options();
		 //Options(MessageLoop::Type type, size_t size);
		 Options(const Options& other);
		 ~Options();
		 
		 //MessageLoop::Type message_loop_type = MessageLoop::TYPE_DEFAULT;
		 MessagePumpFactory message_pump_factory;

		 size_t stack_size = 0;

		 ThreadPriority priority = ThreadPriority::NORMAL;

		 bool joinable = true;
	 }; 

	 explicit Thread(const std::string& name);
	 
	 ~Thread() OVERRIDE;


	 bool Start();

	 bool StartWithOptions(const Options& options);

	 // 等待，知道线程开始.
	 bool WaitUntilThreadStarted() const;

	 void Stop();

	 void StopSoon();

	 //void DetachFromSequence();
	 MessageLoop* message_loop() const {
		 return message_loop_;
	 }

	 PlatformThreadId GetThreadId() const;

	 PlatformThreadHandle GetThreadHandle() ;

	 bool IsRunning() const;

 protected:
	 virtual void Init() {}

	 virtual void Run(RunLoop* run_loop);
	 
	 virtual void CleanUp() {}

	 //static void SetThreadWasQuitProperly(bool flag);
	 //static void GetThreadWasQuitProperly();

	 void SetMessageLoop(MessageLoop* message_loop);

	 bool using_external_message_loop() const {
		 return using_external_message_loop_;
	 }

 private:
	 // PlatformThread::Delegate methods.
	 void ThreadMain() OVERRIDE;

	 void ThreadQuitHelper();

	 //void ThreadQuitHelper();

	 bool joinable_ = true;

	 bool stopping_ = false;

	 bool running_ = false;
	 mutable std::mutex running_mutex_;

	 std::thread thread_;
	 bool is_thread_valid_ = false;
	 mutable std::mutex thread_mutex_;

	 PlatformThreadId id_= kInvalidThreadId;
	 mutable std::condition_variable id_event_;

	 MessageLoop* message_loop_ = nullptr;
	 RunLoop* run_loop_ = nullptr;

	 bool using_external_message_loop_ = false;

	 const std::string name_;

	 mutable std::condition_variable start_event_;

	 mutable std::mutex cond_var_mutex_;

	 DISALLOW_COPY_AND_ASSIGN(Thread);
};

}		// namespace base.


#endif // !BASE_THREADING_THREAD_H
