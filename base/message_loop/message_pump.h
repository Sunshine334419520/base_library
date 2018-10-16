/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: message_pump.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_H
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_H

#include <chrono>

#include "base/base_export.h"

namespace base {

class BASE_EXPORT MessagePump {
 public:
	 class BASE_EXPORT Delegate {
	  public:
		  virtual ~Delegate() {}

		  virtual bool DoWork() = 0;

		  virtual bool DoDelayedWork(
			  std::chrono::milliseconds& next_delayed_work_time) = 0;

		  virtual bool DoIdleWork() = 0;
	 };

	 MessagePump();
	 virtual ~MessagePump();

	 virtual void Run(Delegate* delegate) = 0;

	 virtual void Quit() = 0;

	 virtual void ScheduleWork() = 0;

	 virtual void ScheduleDelayedWork(
		 const std::chrono::milliseconds& delayed_time_work) = 0;

	 virtual void SetTimerSlack();
};
}

#endif // !BASE_MESSAGE_LOOP_MESSAGE_PUMP
