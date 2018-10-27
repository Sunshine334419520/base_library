/**
* @Author: YangGuang
* @Date:   2018-10-20
* @Email:  guang334419520@126.com
* @Filename: task_runner.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_TASK_RUNNER_H
#define BASE_TASK_RUNNER_H

#include <stddef.h>

#include <chrono>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/callback.h"
#include "base/location.h"

namespace base {

struct TaskRunnerTraits;

// 一个TaskRunner对象是一个用来运行posted task的对象，TaskRunner提供了
// 一个可以运行每一个task的方法, TaskRunner 提供一个非常weak的保证，在
// 什么时候这个task运行，
// guarantees: 
//
//  - posting的task不会同步运行，没有Post*Task方法到立即运行task
//  
//  - 增加的delay只能在任务运行的时候延迟， 增加这个delay不可能影响到正在
//    运行的任务， 它可以使task更晚的执行，但是它不能够让task过早的运行. 
//
// TaskRunner 不保证posted task按照顺序执行，是否重复，是否运行在一个
// 制定的线程上，这些它都不能保证，也不保证任务之间在一个memory model上
// 共享数据. （换一句话说，你应该使用你自己的synchronization/locking.
// 如果你需要在任务之间共享数据)
// 
// 实例化TaskRunner必须时安全的调用在每一个线程上面，所以一般使用
// std::unqiue_ptr or std::shared_ptr
//
// Some theoretical implementations of TaskRunner: 
//  - 一个TaskRunner使用一个thread pool来运行posted tasks.
//
//  - 一个TaskRunner对于每个task，用一个non-jojinable thread去运行
//    它们，然后立即退出.
//
//  - 一个TaskRunner应该用一个list来保存posted tasks，并且提供一个方法
//    Run() 来随机的顺序运行每一个可以运行的任务. 
class BASE_EXPORT TaskRunner {
 public: 

	 // Posts 这个给予的任务到运行， 如果任务可能在将来的某个时刻执行，返回
	 // ture，如果任务肯定不会运行则返回false.
     bool PostTask(const Location& from_here,
                   Closure Task);
    
	 // 这个像PostTask一样，但是通过这个函数posted的task只会在延迟delay时间，
	 // 才会运行
     virtual bool PostDelayedTask(const Location& from_here,
                                 Closure Task,
                                 std::chrono::milliseconds delay) = 0;
                                 
	 // 如果返回true，代表实在当前序列，或者说是绑定到的当前线程. 
     virtual bool RunsTasksInCurrentSequence() = 0;

	 // Posts |task| 到当前的TaskRunner，在任务运行完成之后，|reply| 会发送到
	 // 调用这个PostTaskAndReply()的线程上， |task| and |reply|都需要保证在
	 // 调用PostTaskAndReplay()线程上删除
	 // See the following pseudo-code: 
	 //
	 // class DataBuffer {
	 //      void AddData(void* buf, size_t length);
	 //      ...
	 //  };
	 //
	 // class DataLoader {
	 //  public:
	 //      void GetData() {
	 //          std::shared_ptr<DataBuffer> buffer = new DataBuffer();
	 //          target_thread_.task_runner()->PostTaskAndReply(
	 //              FROM_HERE,
	 //              std::bind(&DataBuffer::AddData, buffer),
	 //              std::bind(&DdataLoader::OndataReceived, std::weak_ptr<DataBuffer>(buffer)));
	 //      }
	 //   private: 
	 //       void OnDataReceived(std::shared_ptr<DataBuffer> buffer) {
	 //    
	 //       }
	 // };
	 //
	 // Things to notice:
	 //   * 任务的结果是通过task和reply的共享参数DataBuffer绑定来实现的
	 //   * 这个DataLoader对象没有特殊的线程安全. 
	 //   * 由于使用了std::weak_ptr所以在任务运行的时候删除DataLoader是不会影响到
	 //     任务的接受的，weak_ptr发现被删除了就不会发送回复任务了.    
     bool PostTaskAndReplay(const Location& from_here,
                           Closure task,
                           Closure reply);
                           
 protected: 
     friend struct TaskRunnerTraits;
     TaskRunner();
     virtual ~TaskRunner();

	 // 在对象应该destroyed的时候调用这个函数， 这个函数只会做简单的deletes |this|,
	 // 如果你想要做更多的操作，比如删除在指定的线程上，那么就请重载它.
     virtual void OnDestruct() const;
};

struct BASE_EXPORT TaskRunnerTraits {
    static void Destruct(const TaskRunner* task_runner);
};

}       // namespace base
    
#endif // BASE_TASK_RUNNER_H