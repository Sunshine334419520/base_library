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

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macro.h"

namespace base {

namespace internal {

class BaseTimerTaskInternal;

// 这个类包装了TaskRunner::PostDelayedTask来管理延迟和重复任务. 不要直接
// 使用这个类， 而是使用OneShotTimer, RepeatingTimer, RetainingOneShotTimer.
class BASE_EXPORT TimerBase {
    // 这两个构造函数代表是一个 one-shot或者repeating, 在开始之前必须要设置
    // task， |retain_user_task|在这个user_task运行完成还保留就为true,
    // 如果这个|tick_clock|是提供了，那么在调度任务的时候使用的就是它，而不是
    // std::chrono::system_clock()::now().
    TimerBase(bool retain_user_task, bool is_repeating);
    TimerBase(bool retain_user_task,
              bool is_repeating,
              const TickClock* tick_clock);
};

}   // internal.

}   // base.


#endif