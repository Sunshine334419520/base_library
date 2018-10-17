/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: imcoming_task_queue.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H
#define BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H

#include <chrono>
#include <mutex>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macor.h"
#include "base/pending_task.h"

namespace base {

class MessageLoop;
class PostTaskTest;

namespace internal {

// Implements a queue of tasks posted to the message loop running on the current
// thread. This class takes care of synchronizing posting tasks from different
// threads and together with MessageLoop ensures clean sutdown.
class IncomingTaskQueue {
 public:
	 // 提供一个用于读和删除的队列虚基类.
	 class ReadAndRemoveOnlyQueue {
	  public:
		  ReadAndRemoveOnlyQueue() = default;
		  virtual ~ReadAndRemoveOnlyQueue() = default;


		  // 如果HasTasks() 返回true，则返回下一个任务.
		  virtual const PendingTask& Peek() = 0;

		  // 如果HasTasks() 返回true，则删除并且返回下一个任务.
		  virtual PendingTask Pop() = 0;

		  // 队列中是否有任务
		  virtual bool HasTasks() = 0;

		  // 删除所有任务.
		  virtual void Clear() = 0;
	  private:
		  DISALLOW_COPY_AND_ASSIGN(ReadAndRemoveOnlyQueue);
	 };

	 // 提供一个读写任务队列.
	 class Queue : public ReadAndRemoveOnlyQueue {
	  public:
		  Queue() = default;
		  ~Queue() = default;

		  // 添加一个任务到队列末尾.
		  virtual void Push(PendingTask pending_task) = 0;
		  
	  private:
		  DISALLOW_COPY_AND_ASSIGN(Queue);
	 };

	 explicit IncomingTaskQueue(MessageLoop* message_loop);

	 // 添加一个任务到incoming queue. 所有的任务都需要通过AddToIncomingQueue() or
	 // TryAddToIncomingQueue()，多个不同的线程可以同时提交.
	 // 如果成功返回true, 否则放回false，任务的所有权会被转移到调用的方法.
	 bool AddToIncomingQueue(const Location& from_here,
							 Closure task,
							 std::chrono::milliseconds delay,
							 Nestable nestable);

	 // Returns true if the message loop is "idle".
	 bool IsIdleForTesting();

	 // 将this从父消息循环断开.
	 void WillDestroyCurrentMessageLoop();

	 // 这个函数应该调用在message loop 已经准备好scheduling work时.
	 void StartScheduling();

	 // Runs |pending_task|.
	 void RunTask(PendingTask* pending_task);

	 // 下面这三个队列，都是用于message loop中保存需要运行的消息的对应队列.
	 // 一个保存着普通的任务队列, 一个保存着延迟任务队列, 一个保存着闲置任务队列.
	 class TriageQueue : public ReadAndRemoveOnlyQueue {
	  public:
		  TriageQueue(IncomingTaskQueue* outer);
		  ~TriageQueue() OVERRIDE;

		  // ReadAndRemoveOnlyQueue:
		  // 如果队列时空的(调用clear除外), 那么下列方法就会从incoming 队列从新加载.
		  const PendingTask& Peek() OVERRIDE;
		  PendingTask Pop() OVERRIDE;

		  bool HasTasks() OVERRIDE;
		  void Clear() OVERRIDE;

	  private:
		  // 如果队列为空从incoming queue 重新加载.
		  void ReloadFromIncomingQueueIfEmpty();

		  IncomingTaskQueue* const outer_;
		  TaskQueue queue_;

		  DISALLOW_COPY_AND_ASSIGN(TriageQueue);
	 };

	 class DelayedQueue : public Queue {
	  public:
		  DelayedQueue(IncomingTaskQueue* outer);
		  ~DelayedQueue() OVERRIDE;

		  // Queue:
		  const PendingTask& Peek() OVERRIDE;
		  PendingTask Pop() OVERRIDE;

		  bool HasTasks() OVERRIDE;
		  void Clear() OVERRIDE;
		  void Push(PendingTask pending_task) OVERRIDE;

	  private:
		  IncomingTaskQueue* const outer_;
		  DelayedTaskQueue queue_;

		  DISALLOW_COPY_AND_ASSIGN(DelayedQueue);
	 };

	 class DeferredQueue : public Queue {
	  public:
		  DeferredQueue(IncomingTaskQueue* outer);
		  ~DeferredQueue() OVERRIDE;

		  // Queue:
		  const PendingTask& Peek() OVERRIDE;
		  PendingTask Pop() OVERRIDE;

		  bool HasTasks() OVERRIDE;
		  void Clear() OVERRIDE;
		  void Push(PendingTask pending_task) OVERRIDE;
		  
	  private:
		  IncomingTaskQueue* const outer_;
		  TaskQueue queue_;

		  DISALLOW_COPY_AND_ASSIGN(DeferredQueue);
	 };

	 virtual ~IncomingTaskQueue();

	 // 添加一个任务到 incoming queue. 调用者可以继续保持这个pending_task,
	 // 但是这个函数将会重置pending_task->task的值，因为这个函数需要保证这个
	 // pending_task->task的生命周期不会超过这个函数.
	 bool PostPendingTask(PendingTask* pending_task);

	 // 这个函数作真正的posting a pending task, 如果返回true，这个调用一你应该在
	 // 这个message loop 上面调用ScheduleWork() .
	 bool PostPendingTaskLockRequired(PendingTask* pending_task);

	 // 加载任务，从incoming queue到work queue
	 // 返回|work_queue|中需要高分辨率计时器的任务数量。
	 int ReloadWorkQueue(TaskQueue* work_queue);

	 // 如果设置为true，表示只要接受到任务就会调用ScheduleWork(), 
	 // 只要incoming queue 不是空.
	 const bool always_schedule_work_;

	 TriageQueue triage_tasks_;

	 DelayedQueue delayed_tasks_;

	 DeferredQueue deferred_tasks_;

	 // 高分辨率任务的个数.
	 int pending_high_res_tasks_ = 0;

	 // 用于保护|message_loop_|
	 std::mutex message_loop_lock_;

	 // 指向拥有this的消息循环.
	 MessageLoop* message_loop_;

	 // 用于同步访问该行一下的所有数据.
	 std::mutex incoming_queue_lock_;

	 // Number of tasks that require high resolution timing.This value is kept
	 // so that ReloadWorkQueue() completes in constant time.
	 int high_res_task_count_ = 0;

	 // 这个队列里面保存的任务是还没有放到message loop 中的.
	 TaskQueue incoming_queue_;

	 // 如果应该接受新的任务就为true.
	 bool accept_new_tasks_ = true;

	 // 用于延迟任务的下一个序列号.
	 int next_sequence_num_ = 0;

	 // 如果我们的message loop 是已经scheduled并且不需要再一次scheduled时为true.
	 // 直到为空时在重新加载.
	 bool message_loop_scheduled_ = false;

	 // 直到StartScheduling()调用前都为false.
	 bool is_ready_for_schedulig_ = false;

	 DISALLOW_COPY_AND_ASSIGN(IncomingTaskQueue);
};

}				// namespace internal.

}				// namespace base.


#endif // BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H