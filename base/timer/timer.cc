/**
* @Author: YangGuang
* @Date:   2018-11-4
* @Email:  guang334419520@126.com
* @Filename: timer.cc
* @Last modified by:  YangGuang
*/
#include "base/timer/timer.h"

#include <stddef.h>
#include <utility>

#include "base/logging.h"
#include "base/threading/platform_thread.h"
#include "base/sequenced_task_runner_handle.h"

namespace base {
namespace internal {

class BaseTimerTaskInternal {
 public:
	 explicit BaseTimerTaskInternal(TimerBase* timer) : timer_(timer) {}

	 ~BaseTimerTaskInternal() {
		 if (timer_)
			 timer_->AdandonAndStop();
	 }

	 void Run() {
		 if (!timer_)
			 return;
		 timer_->scheduled_task_ = nullptr;

		 TimerBase* timer = timer_;
		 timer_ = nullptr;
		 timer->RunScheduledTask();
	 }

	 void Abandon() { timer_ = nullptr; }

 private:
	 TimerBase* timer_;

	 DISALLOW_COPY_AND_ASSIGN(BaseTimerTaskInternal);
};



TimerBase::TimerBase(bool retain_user_task, bool is_repeating)
	: TimerBase(retain_user_task, is_repeating, nullptr) {}

TimerBase::TimerBase(bool retain_user_task, 
					 bool is_repeating,
					 const TickClock* tick_clock)
	: scheduled_task_(nullptr),
	  is_repeating_(is_repeating),
	  retain_user_task_(retain_user_task),
	  tick_clock_(tick_clock),
	  is_running_(false) {}

TimerBase::TimerBase(const Location& posted_from,
					 TimeDelta delay,
					 const base::Closure& user_task,
					 bool is_repeating)
	: TimerBase(posted_from, delay, user_task, is_repeating, nullptr) {}

TimerBase::TimerBase(const Location& posted_from, TimeDelta delay, const base::Closure& user_task, bool is_repeating, const TickClock* tick_clock)
	: scheduled_task_(nullptr),
	  posted_from_(posted_from),
	  delay_(delay),
	  user_task_(user_task),
	  is_repeating_(is_repeating),
	  retain_user_task_(true),
	  tick_clock_(tick_clock),
	  is_running_(false) {

}

TimerBase::~TimerBase() {
	AdandonAndStop();
}

bool TimerBase::IsRunning() const {
	return is_running_;
}

TimerBase::TimeDelta TimerBase::GetCurrentDelay() const {
	return delay_;
}

void TimerBase::SetTaskRunner(std::shared_ptr<SequencedTaskRunner> task_runner) {
	DCHECK(!is_running_);
	task_runner_.swap(task_runner);
}

void TimerBase::Start(const Location & posted_from,
					  TimeDelta delay,
					  const base::Closure & user_task) {
	posted_from_ = posted_from;
	delay_ = delay;
	user_task_ = user_task;

	Reset();
}

void TimerBase::Stop() {
	is_running_ = false;

	if (!retain_user_task_)
		user_task_.Reset();
}

void TimerBase::Reset() {
	// user_task必须不为空
	DCHECK(!user_task_.is_null());

	// 如果没有pending task，那么启动一个就返回.
	if (!scheduled_task_) {
		PostNewScheduledTask(delay_);
		return;
	}

	if (delay_ > std::chrono::milliseconds(0))
		desired_run_time_ = Now() + delay_;
	else
		desired_run_time_ = std::chrono::milliseconds(0);

	// We can use the existing scheduled task if it arrives before the new
	// |desired_run_time_|.
	if (desired_run_time_ >= scheduled_run_time_) {
		is_running_ = true;
		return;
	}

	// We can't reuse the |scheduled_task_|, so abandon it and post a new one.
	AdandonScheduledTask();
	PostNewScheduledTask(delay_);

}

std::chrono::milliseconds TimerBase::Now() const {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
}

void TimerBase::PostNewScheduledTask(TimeDelta delay) {
	DCHECK(!scheduled_task_);
	is_running_ = true;
	scheduled_task_ = new BaseTimerTaskInternal(this);
	if (delay > TimeDelta(0)) {
		GetTaskRunner()->PostDelayedTask(posted_from_,
			base::BindOnceClosure(&BaseTimerTaskInternal::Run, scheduled_task_),
		 delay);
		scheduled_run_time_ = desired_run_time_ = Now() + delay;
	}
	else {
		GetTaskRunner()->PostTask(posted_from_,
								  base::BindOnceClosure(&BaseTimerTaskInternal::Run,
								  scheduled_task_));
	}
}

std::shared_ptr<SequencedTaskRunner> TimerBase::GetTaskRunner() {
	return task_runner_.get() ? task_runner_ : SequencedTaskRunnerHandle::Get();
}

void TimerBase::AdandonScheduledTask() {
	if (scheduled_task_) {
		scheduled_task_->Abandon();
		scheduled_task_ = nullptr;
	}
}

void TimerBase::RunScheduledTask() {
	if (!is_running_)
		return;

	if (desired_run_time_ > scheduled_run_time_) {
		auto now = Now();

		if (desired_run_time_ > now) {
			PostNewScheduledTask(desired_run_time_ - now);
			return;
		}
	}

	base::Closure task = user_task_;

	if (is_repeating_)
		PostNewScheduledTask(delay_);
	else
		Stop();

	task.Run();
}

}	// namespace internal.

void OneShotTimer::FireNow() {
	DCHECK(!task_runner_);
	DCHECK(IsRunning());

	Closure task = user_task();
	Stop();
	DCHECK(user_task().is_null());
	std::move(task).Run();
}
}