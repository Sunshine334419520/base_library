/**
* @Author: YangGuang
* @Date:   2018-10-23
* @Email:  guang334419520@126.com
* @Filename: sequenced_task_runner.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_SEQUENCED_TASK_RUNNER_H
#define BASE_SEQUENCED_TASK_RUNNER_H

#include <memory>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/task_runner.h"
#include "base/sequenced_task_runner_helpers.h"

namespace base {

// SequencedTaskRunner 是一个TaskRunner的子类，它提供了附加的保证，它保证
// 任务开始在有序的队列上，并且还保证任务按顺序完成，换一句话说就是，一个任务
// 接着一个任务运行.
//
// Summary
// -------
// Non-nested tasks 和具有同样延迟的 task 将顺序运行在一个FIFO上.
//
// Detailed guarantees
// -------------------
//
// SequencedTaskRunner 也提供了几个自己的附加方法到posting non-nestable
// tasks. 通常，TaskRunner的实现可能公开任务运行方法，这些方法本身可以在
// 任务内部调用。不稳定任务是保证不会从已经运行的任务中运行的任务。相反，
// nestable任务(默认)是可以在已经运行的任务中运行的任务。
//
// The guarantees of SequencedTaskRunner are as follows:
//	
//	 - 给了2个task T1和T2， 如果T2开始在T1开始之后:
//		
//		* T2 posted在T1的后面;
//      * T2 的delay是大于或者等于T1;
//      * T2 是一个 non-nestable 或者 T1 是一个 nestable.
//
// SequencedTaskRunner 是不会保证tasks运行在一个单一的专用线程上，如果你
// 需要运行在一个专用的线程上，那么请查看SingleThreadTaskRunner(在
// single_thread_task_runner.h).

class BASE_EXPORT SequencedTaskRunner : public TaskRunner {
 public:
	 //  这两个PostNonNestable*Task 方法跟这个TaskRunner的nestable差不多，
	 // 但是这个保证提交的任务不会运行在一个已经运行的任务上.
	 //
	 // PostNonNestable任务只会比规定的运行的迟，绝对不会早.
	 bool PostNonNestableTask(const Location& from_here, Closure task);

	 virtual bool PostNonNestableDelayedTask(const Location& from_here,
											 Closure task,
											 std::chrono::milliseconds delay) = 0;

	 // 提交一个non-nestable task来删除一个给定的对象, 如果返回true，
	 // 那么这个对象可能在之后被删除，如果返回false，那么这个对象肯定
	 // 不会删除.
	 template <typename T>
	 bool DeleteSoon(const Location& from_here, const T* object) {
		 return DeleteOrReleaseSoonInternal(from_here, &DeleteHelper<T>::DoDelete,
											object);
	 }

	 template <typename T>
	 bool DeleteSoon(const Location& from_here, std::unique_ptr<T> object) {
		 return DeleteSoon(object.release());
	 }

	 // 提交一个non-nestable task来release一个给定的对象，如果返回true，
	 // 那么这个对象可能咋之后release，如果返回false，那么这个对象肯定
	 // 不会release.
	 template <typename T>
	 bool ReleaseSoon(const Location& from_here, const T* object) {
		 return DeleteOrReleaseSoonInternal(from_here, &ReleaseHelper<T>::DoRelease,
											object);
	 }

 protected:
	 ~SequencedTaskRunner() OVERRIDE;
 private:
	 bool DeleteOrReleaseSoonInternal(const Location& from_here,
									  void(*deleter)(const void*),
									  const void* object);
};

// Sample usage with std::unique_ptr :
// std::unique_ptr<Foo, base::OnTaskRunnerDeleter> ptr(
//     new Foo, base::OnTaskRunnerDeleter(my_task_runner));
struct BASE_EXPORT OnTaskRunnerDeleter {
	explicit OnTaskRunnerDeleter(std::shared_ptr<SequencedTaskRunner> task_ruuner);
	~OnTaskRunnerDeleter();

	OnTaskRunnerDeleter(OnTaskRunnerDeleter&&);
	OnTaskRunnerDeleter& operator=(OnTaskRunnerDeleter&&);

	template <typename T>
	void operator()(const T* ptr) {
		if (ptr)
			task_runner_->DeleteSoon(FROM_HERE, ptr);
	}

	std::shared_ptr<SequencedTaskRunner> task_runner_;
};

}	// namespace base.

#endif // !BASE_SEQUENCED_TASK_RUNNER_H
