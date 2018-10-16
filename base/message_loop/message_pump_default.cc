/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: message_pump_default.h
* @Last modified by:  YangGuang
*/
#include "base/message_loop/message_pump_default.h"

namespace base {

MessagePumpDefault::MessagePumpDefault() = default;

MessagePumpDefault::~MessagePumpDefault() = default;

// MessagePump methods:
void MessagePumpDefault::Run(Delegate* delegate) {
	for (;;) {
		bool did_work = delegate->DoWork();
		if (!keep_running_)
			break;

		did_work |= delegate->DoDelayedWork(delayed_work_time_);
		if (!keep_running_)
			break;

		if (did_work)
			// 作了延迟任务或者work.
			continue;

		// 没有做工作，就去做闲置的工作
		did_work = delegate->DoIdleWork();
		if (!keep_running_)
			break;

		// 做了闲置工作，contiune.
		if (did_work)
			continue;

		std::unique_lock<std::mutex> lock(mutex_);
		if (delayed_work_time_.count() == 0) {
			event_.wait(lock);
		}
		else {
			event_.wait_until(lock, &std::_To_xtime(delayed_work_time_));
		}

	}
}

void MessagePumpDefault::Quit() {
	keep_running_ = false;
}

void MessagePumpDefault::ScheduleWork() {
	std::lock_guard<std::mutex> lock(mutex_);
	event_.notify_one();
}

void MessagePumpDefault::ScheduleDelayedWork(
	const  std::chrono::milliseconds& delayed_time_work) {
	delayed_work_time_ = delayed_time_work;
}

}