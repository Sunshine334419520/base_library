/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: message_pump_default.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H

#include <chrono>
#include <atomic>
#include <condition_variable>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/message_loop/message_pump.h"

namespace base {

class BASE_EXPORT MessagePumpDefault : public MessagePump {
 public:
	 MessagePumpDefault();
	 ~MessagePumpDefault() OVERRIDE;

	 // MessagePump methods:
	 void Run(Delegate* delegate) OVERRIDE;
	 void Quit() OVERRIDE;
	 void ScheduleWork() OVERRIDE;
	 void ScheduleDelayedWork(
		 const  std::chrono::milliseconds& delayed_time_work) OVERRIDE;

 private:
	 // This is flag is set to false when Run should return.
	 std::atomic<bool> keep_running_ = true;
	 
	 // Used to sleep until there is more work to do.
	 std::condition_variable event_;

	 std::mutex mutex_;

	 // the time at which we should call DodelayedWork.
	 std::chrono::milliseconds delayed_work_time_;

	 DISALLOW_COPY_AND_ASSIGN(MessagePumpDefault);
	 
};

}	// namespace base.

#endif // !BASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H
