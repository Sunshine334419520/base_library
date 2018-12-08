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
			// 在这里原本是准备使用wait_until的，可是由于C++11的wait_until接受的参数
			// 是time_point, 而我们保存的delayed_work_time_确实std::chrno::duration
			// 类型，这两个类型之间的转换我弄了半天也没发现如何转换,所有改成了wait_for,
			// 改成wait_for的话就需要计算等待的时长，而不是运行的时间.
			auto now = std::chrono::system_clock::now();		
			auto wait_time = 
				delayed_work_time_ - std::chrono::duration_cast<
				std::chrono::milliseconds>(now.time_since_epoch());
			if (wait_time.count() <= 0)
				continue;
			//event_.wait_until(lock, now);
			event_.wait_for(lock, wait_time);
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