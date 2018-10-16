/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: imcoming_task_queue.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H
#define BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H


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
		  //virtual const PendingTask& Peek() = 0;

	 };
};

}				// namespace internal.

}				// namespace base.


#endif // BASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H