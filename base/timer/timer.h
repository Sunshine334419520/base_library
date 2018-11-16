/**
* @Author: YangGuang
* @Date:   2018-11-4
* @Email:  guang334419520@126.com
* @Filename: timer.h
* @Last modified by:  YangGuang
*/

// OneShotTimer, RepeatingTimer and RetainingOneShotTimer 都提供了
// 非常简单都timer API. 像这个名字一样，OneShotTimer 只在制定的延迟时间
// 之后调用一次。
// RepeatingTimer 在一个指定都时间间隔内反复都执行指定都任务.
// RetainingShotTimer 像这个OneShotTimer差不多，但是它在任务运行完成之
// 后还保留下了这个任务， 你也可以重新开始用一个新的任务.
//
// OneShotTimer, RepeatingTimer, RetainingShotTimer它们在离开自己的
// 作用域时任务都会被取消，要想避免这样的情况发生，将他们放在类的成员变量是一
// 个非常好的选择. 
//
// Sample RepeatingTimer usage:
//      class MyClass {
//       public:
//          void StartDoingStuff() {
//              timer.Start(FROM_HERE, TimerDelta::FromSeconds(1),
//                          this, &MyClass::DoStuff);
//          }
//          void StopDoingStuff() {
//              timer.Stop();
//          }
//       private:
//          void DoStuff() {
//              // This method is called every second to do stuff.
//              ...
//          }
//          
//          base::RepeatingTimer timer_;
//      };
//
// Timers还提供了一个Reset方法，这允许您轻松地延迟计时器事件，直到计时器延迟
// 再次通过, 如果0.5秒已经过去了，那么在|timer_|上调用Reset将使DoStuff延迟
// 1秒. 换句话说，这个Reset就像是一个非常快速的调用的Stop()然后再调用Start().
//
// 这些 APIs 都不是thread asfe. 所有的方法都必须调用在一个同样的序列上，除了
// destructor 和 SetTaksRunner(). 
// - 当计时器不运行且没有计划任务活动时，可以从任意序列调用析构函数，比如，在这
// 个start永远都不会被调用或者是在调用了AbandonAndStop()之后. 
// - 在这个timer还没有被start的时候，这个SetTaskRunner()可以调用在任何的序
// 列
//
// 默认情况下，在这个timer上开始的任务都将运行在同一个scheduled task上面，
// 但是可以在Start()之前改变通过这个SetTaskRunner(). 

#ifndef BASE_TIMER_TIMER_H
#define BASE_TIMER_TIMER_H

#include <memory>
#include <chrono>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macor.h"
#include "base/location.h"
#include "base/sequenced_task_runner.h"

namespace base {

class TickClock;

namespace internal {

class BaseTimerTaskInternal;


// 这个类包装了TaskRunner::PostDelayedTask来管理延迟和重复任务. 不要直接
// 使用这个类， 而是使用OneShotTimer, RepeatingTimer, RetainingOneShotTimer.
class BASE_EXPORT TimerBase {
 public:
	using TimeDelta = std::chrono::milliseconds;
	// 这两个构造函数代表是一个 one-shot或者repeating, 在开始之前必须要设置
	// task， |retain_user_task|在这个user_task运行完成还保留就为true,
	// 如果这个|tick_clock|是提供了，那么在调度任务的时候使用的就是它，而不是
	// std::chrono::system_clock()::now().
    // std::chrono::system_clock()::now().
    TimerBase(bool retain_user_task, bool is_repeating);
    TimerBase(bool retain_user_task,
              bool is_repeating,
              const TickClock* tick_clock);

	// 构造一个retained task timer信息, 如果|tick_clock|是又提供了，那么就
	// 使用它而不是使用std::chrono::system_clock()::now().
	TimerBase(const Location& posted_from,
			  TimeDelta delay,
			  const base::Closure& user_task,
			  bool is_repeating);
	TimerBase(const Location& posted_from,
			  TimeDelta delay,
			  const base::Closure& user_task,
			  bool is_repeating,
			  const TickClock* tick_clock);

	virtual ~TimerBase();

	// 如果timer正在运行，那么就返回true.
	bool IsRunning() const;

	// 获得当前timer延迟时间.
	TimeDelta GetCurrentDelay() const;

	// 设置这个task runner，告诉这个timer这个task应该如何被调度，这个方法必须调用
	// 在任何task都还没有被scheduled之前，如果|task_runner|运行任务的序列与拥有这
	// 个计时器的序列不同，那么当计时器触发时|user_task_|将被发布到它(注意，这意味
	// 着|user_task_|可以在~Timer()之后运行并且应该支持它). 
	virtual void SetTaskRunner(std::shared_ptr<SequencedTaskRunner> task_runner);

	// 用给予的delay来开始这个timer，如果这个timer已经处于一个运行状态，那么这个
	// |user_task|将代替正砸执行的任务接着运行.
	virtual void Start(const Location& posted_from,
					   TimeDelta delay,
					   const base::Closure& user_task);

	/*
	// 用给的这个delay来开始这个timer，如果这个timer已经处于一个运行状态，那么这
	// 个|Receiver::*method|将代替这个正在执行的任务接着运行.
	template <class Receiver>
	void Start(const Location& posted_from,
			   TimeDelta delay,
			   Receiver* receiver,
			   void(Receiver::*method())) {
		Start(posted_from, delay,
			  base::BindOnceClosure(method, receiver));
	}
	*/

	// 调用这个方法来定制并且取消这个timer.
	virtual void Stop();

	// 停止运行任务(如果有)并放弃计划任务(如果有).
	void AdandonAndStop() {
		AdandonScheduledTask();

		Stop();
	}

	// 调用这个方法将会重新刷新这个timer delay，这个|user_task_|必须是已经设置，
	// 如果这个timer还没有运行，那么调用这个函数将会用一个已有的task开始.
	virtual void Reset();

	const base::Closure& user_task() const { return user_task_; }
	const std::chrono::milliseconds desired_run_time() const {
		return desired_run_time_;
	}

 protected:
	 std::chrono::milliseconds Now() const;

	 void set_user_task(const Closure& task) { user_task_ = task; }
	 void set_desired_run_time(std::chrono::milliseconds desired) {
		 desired_run_time_ = desired;
	 }
	 void set_is_running(bool running) { is_running_ = running; }

	 const Location& posted_from() const { return posted_from_; }

	 std::shared_ptr<SequencedTaskRunner> task_runner_;

 private:
	 friend class BaseTimerTaskInternal;

	 // 分配一个新的|scheduled_task_|，并以给定的|延迟|将其发布到当前序列上。
	 // |scheduled_task_|必须是null. |scheduled_run_time_| and 
	 // |desired_run_time_| 都会被刷新用now() +  delay.
	 void PostNewScheduledTask(TimeDelta delay);

	 // 返回应该调度任务的任务运行器。如果相应的|task_runner_|字段为null，则返回
	 // 当前序列的task runner。
	 std::shared_ptr<SequencedTaskRunner> GetTaskRunner();
	 
	 //禁用|scheduled_task_|，并放弃它，这样它就不再引用这个对象。
	 void AdandonScheduledTask();

	 void RunScheduledTask();

	 // 在这个不为null的时候, |scheduled_task_|会调用RunscheduledTask().
	 BaseTimerTaskInternal* scheduled_task_;

	 // Location in user code.
	 Location posted_from_;
	 // Delay requested by user.
	 TimeDelta delay_;
	 // |user_task| is what the user wants to be run at |desired_run_time|.
	 base::Closure user_task_;

	 // |scheduled_task|的预计发射时间, 如果这个必须立即执行，那么这个时间可能是0.
	 std::chrono::milliseconds scheduled_run_time_;

	 // 这个是|user_task_|想要运行的时间, 用户可以在随时更新这个时间, 即使它们之前
	 // 请求的还没有执行. 如果|desired_run_time_|大于|scheduled_run_time_|,将发
	 // 布一个延续任务，等待剩余的时间.这允许我们重用挂起的任务，这样当用户代码过度
	 // 停止并启动计时器时，就不会用孤立的任务淹没延迟的队列。如果任务必须立即运行，
	 // 那么这个时间可能是“零”时间刻度。
	 std::chrono::milliseconds desired_run_time_;

	 // 如果这个timer是一个重复执行任务就为true.并且retain_user_task_一样要是true.
	 const bool is_repeating_;

	 // 如果任务执行完成后，还保留任务的话就设置为true.
	 const bool retain_user_task_;

	 const TickClock* const tick_clock_;

	 bool is_running_;

	 DISALLOW_COPY_AND_ASSIGN(TimerBase);
};

}   // internal.

class BASE_EXPORT OneShotTimer : public internal::TimerBase {
 public:
	 OneShotTimer() : OneShotTimer(nullptr) {}
	 explicit OneShotTimer(const TickClock* tick_clock)
		 : internal::TimerBase(false, false, tick_clock) {}

	 // 立即运行任务.
	 void FireNow();

};

class RepeatingTimer : public internal::TimerBase {
 public:
	 RepeatingTimer() : RepeatingTimer(nullptr) {}
	 explicit RepeatingTimer(const TickClock* tick_clock)
		 : internal::TimerBase(true, true, tick_clock) {}

	 RepeatingTimer(const Location& posted_from,
					TimeDelta delay,
					base::Closure user_task)
		 : internal::TimerBase(posted_from, delay, std::move(user_task), true) {}
	 RepeatingTimer(const Location& posted_from,
					TimeDelta delay,
					base::Closure user_task,
					const TickClock* tick_clock)
		 : internal::TimerBase(posted_from,
							   delay,
							   std::move(user_task),
							   true,
							   tick_clock) {}
};

class RetainingOneShotTimer : public internal::TimerBase {
public:
	RetainingOneShotTimer() : RetainingOneShotTimer(nullptr) {}
	explicit RetainingOneShotTimer(const TickClock* tick_clock)
		: internal::TimerBase(true, false, tick_clock) {}

	RetainingOneShotTimer(const Location& posted_from,
						  TimeDelta delay,
						  base::Closure user_task)
		: internal::TimerBase(posted_from, delay, std::move(user_task), false) {}
	RetainingOneShotTimer(const Location& posted_from,
						  TimeDelta delay,
						  base::Closure user_task,
						  const TickClock* tick_clock)
		: internal::TimerBase(posted_from,
							  delay,
							  std::move(user_task),
							  false,
							  tick_clock) {}
};

}   // base.


#endif