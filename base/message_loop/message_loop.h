/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: message_loop.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_H

#include <memory>
#include <queue>
#include <string>
#include <chrono>
#include <list>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macor.h"
#include "base/message_loop/incoming_task_queue.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/message_pump_default.h"
#include "base/pending_task.h"
#include "base/threading/platform_thread.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/message_loop/message_loop_task_runner.h"



namespace base {

class BASE_EXPORT MessageLoop : public MessagePump::Delegate {
 public:
	 // A MessageLoop has a particular type, which indicates the set of
	 // asynchronous events it may process in addition to tasks and timers.
	 //
	 // TYPE_DEFAULT
	 //   This type of ML only supports tasks and timers.
	 //
	 // TYPE_UI
	 //   This type of ML also supports native UI events (e.g., Windows messages).
	 //   See also MessageLoopForUI.
	 //
	 // TYPE_IO
	 //   This type of ML also supports asynchronous IO.  See also
	 //   MessageLoopForIO.
	 //
	 //
	 // TYPE_CUSTOM
	 //   MessagePump was supplied to constructor.
	 enum Type {
		 TYPE_DEFAULT,
		 TYPE_UI,
		 TYPE_CUSTOM,
		 TYPE_IO,
	 };

	 explicit MessageLoop(Type type = TYPE_DEFAULT);

	 explicit MessageLoop(std::unique_ptr<MessagePump> pump);

	 ~MessageLoop() OVERRIDE;

	 // Returns the MessageLoop object for the current thread, or null if none.
	 static MessageLoop* current();

	 using MessagePumpFactory = std::unique_ptr<MessagePump>();

	 // Creates the default MessagePump based on |type|. Caller owns return
	 // value.
	 static std::unique_ptr<MessagePump> CreateMessagePumpForType(Type type);

	 // A DestructionObserver is notified when the current MessageLoop is being
	 // destroyed.  These observers are notified prior to MessageLoop::current()
	 // being changed to return NULL.  This gives interested parties the chance to
	 // do final cleanup that depends on the MessageLoop.
	 //
	 // NOTE: Any tasks posted to the MessageLoop during this notification will
	 // not be run.  Instead, they will be deleted.
	 //
	 class BASE_EXPORT DestructionObserver {
	  public:
		  virtual void WillDestroyCurrentMessageLoop() = 0;

	  public:
		  virtual ~DestructionObserver();
	 };

	 // Add a DestructionObserver, which will start receiving notifications
	 // immediately.
	 void AddDestructionObserver(
		 std::shared_ptr<DestructionObserver> destruction_observer);

	 // Remove a DestructionObserver.  It is safe to call this method while a
	 // DestructionObserver is receiving a notification callback.
	 void RemoveDestructionObserver(
		 std::shared_ptr<DestructionObserver> destruction_observer);

	 // Deprecated: use RunLoop instead.
	 // Construct a Closure that will call QuitWhenIdle(). Useful to schedule an
	 // arbitrary MessageLoop to QuitWhenIdle.
	 static Closure QuitWhenIdleClosure();

	 // Returns true if this loop is |type|. This allows subclasses (especially
	 // those in tests) to specialize how they are identified.
	 virtual bool IsType(Type type) const;

	 // Returns the type passed to the construcotr.
	 Type type() const { return type_; }

	 // Returns the name of the thread this message loop is bound to. This function
	 // is only valid when this message loop is running, BindToCurrentThread has
	 // already been called and has an "happens-before" relationship with this call
	 // (this relationship is obtained implicitly by the MessageLoop's task posting
	 // system unless calling this very early).
	 std::string GetThreadName() const;

	 // Gets the TaskRunner associated with this message loop.
	 //const std::shared_ptr<SingleThreadTaksRunner>& task_runner();

	 // Sets a new TaskRunner for this message loop. The message loop must already
	 // have been bound to a thred prior to this call, and the task runner must
	 // belong to that thread. Note that changing the task runner will also affect
	 // the ThreadTaskRunnerHandle for the target thread. Must be called on the
	 // thread to which the message loop is bound.
	 //void SetTaskRunner(const std::shared_ptr<SingleThreadTaksRunner>& task_runner);

	 // Clears task_runner() and the ThreadTaskRunnerHandle for the target thread.
	 // Must be called on the thread to which the message loop is bound.
	 void ClearTaskRunnerForTesting();

	 void SetNestableTasksAllowed(bool allowed);
	 bool NestableTasksAllowed() const;

	 // 范围性的允许nestable task.
	 class ScopedNestableTaskAllower {
	  public:
		  explicit ScopedNestableTaskAllower(MessageLoop* loop)
			  : loop_(loop),
			  old_state_(loop_->NestableTasksAllowed()) {
			  loop_->SetNestableTasksAllowed(true);
		  }

	  private:
		  MessageLoop* const loop_;
		  const bool old_state_;
	 };

	 class BASE_EXPORT TaskObserver {
	  public:
		  TaskObserver();

		  virtual void OnBeforeProcessTask(const PendingTask& pending_task) = 0;

		  virtual void OnAfterProcessTask(const PendingTask& pending_task) = 0;

	  protected:
		  virtual ~TaskObserver() = default;
	 };

	 void AddTaskObserver(std::shared_ptr<TaskObserver> task_observer);
	 void RemoveTaskObserver(std::shared_ptr<TaskObserver> task_observer);

	 bool IsIdleForTesting();

	 // 运行一个pending_task 任务.
	 void RunTask(PendingTask* pending_task);

	 void DisallowTaskObservers() { allow_task_observers_ = false; }

 protected:
	 friend internal::IncomingTaskQueue;
	 friend PendingTask;
	 //friend Thread;

	 std::unique_ptr<MessagePump> pump_;
	 
	 using MessagePumpFactoryCallback = 
		 std::function<std::unique_ptr<MessagePump>(void)>;

	 // 这个就是一个一般的protected constructor. 其他的constructors 会调用这个
	 // constructor来进行initialization.
	 // 一个子类可以直接调用这个constructor 来创建一个自定义的message_loop, 这个
	 // 构造函数不会调用BindToCurrentThread, 如果这个constructor 是子类直接调用，
	 // 那么必须在之后bind the message_loop.
	 MessageLoop(Type type, MessagePumpFactoryCallback pump_factory);

	 // 配置不同的成员并且绑定这个message_loop 到当前的thread.
	 void BindToCurrentThread();

	 // 创建一个没有绑定到线程的MessageLoop.
	 // If |type| is TYPE_CUSTOM non-null |pump_factory| must be also given
	 // to create a message pump for this message loop.  Otherwise a default
	 // message pump for the |type| is created.
	 // 调用这个创建一个消息循环是有效的，这个message_loop的BindToCurrentThread()
	 // 方法必须调用在这个message loop 调用Run() 之前。
	 // 在调用BindToCurrentThread() 之前，只有Post*Task()可以调用在这个message loop上.
	 static std::unique_ptr<MessageLoop> CreateUnbound(
		 Type type,
		 MessagePumpFactoryCallback pump_factory);

	 // Sets the ThreadTaskRunnerHandle for the current thread to point to the
	 // task runner for this message loop.
	 void SetThreadTaskRunnerHandle();

	 // RunLoop::Delegate:
	 //void Run(bool application_tasks_allowed) OVERRIDE;
	 //void Quit() OVERRIDE;
	 //void EnsureWorkScheduled() OVERRIDE;

	 // Called to process any delayed non-nestable tasks.
	 bool ProcessNextDelayedNoNestableTask();

	 // 运行pending_task , 如果现在不能够运行，那就添加到deferred task list 
	 // Retrun true if the task was run.
	 bool DeferOrRunPendingTask(PendingTask pending_task);

	 // 删除所有还没有运行的任务，主要用在析构函数.
	 void DeletePendingTasks();

	 // 唤醒message pump. 可以调用在任务的线程上.
	 void SchedueWork();

	 // MessasgePump::Delegate methods:
	 bool DoWork() OVERRIDE;
	 bool DoDelayedWork(
		 std::chrono::milliseconds& next_delayed_work_time) OVERRIDE;
	 bool DoIdleWork() OVERRIDE;

	 const Type type_;

	 // A recent snapshot of Time::Now(), used to check delayed_work_queue_.
	 std::chrono::milliseconds recent_time;


	 std::list<std::shared_ptr<DestructionObserver>> destruction_observers_;

	 // A boolean which prevents unintentional reentrant task execution (e.g. from
	 // induced nested message loops). As such, nested message loops will only
	 // process system messages (not application tasks) by default. A nested loop
	 // layer must have been explicitly granted permission to be able to execute
	 // application tasks. This is granted either by
	 // RunLoop::Type::kNestableTasksAllowed when the loop is driven by the
	 // application or by a ScopedNestableTaskAllower preceding a system call that
	 // is known to generate a system-driven nested loop.
	 bool task_execution_allowed_ = true;

	 // pump_factory_.Run() is called to create a message pump for this loop
	 // if type_ is TYPE_CUSTOM and pump_ is null.
	 MessagePumpFactoryCallback pump_factory_;

	 std::list<std::shared_ptr<TaskObserver>> task_observers_;

	 // 保存着当前正在处理的任务，没有别的意思
	 const PendingTask* current_pending_task_ = nullptr;

	 std::shared_ptr<internal::IncomingTaskQueue> incoming_task_queue_;

	 // 一个我们还没有绑定到thread 上的task runner.
	 std::shared_ptr<internal::MessageLoopTaskRunner> unbound_task_runner_;


	 // 这个task runner 和memssage lopp 关联.
	 std::shared_ptr<SingleThreadTaskRunner> task_runner_;
	 std::unique_ptr<ThreadTaskRunnerHandle> thread_task_runner_handle_;

	 // 绑定到这个消息循环的线程的线程id, 只会在绑定线程到MessageLoop时会初始化一次
	 // 在这之后这个thread_id_永远都不会改变.
	 PlatformThreadId thread_id_ = kInvalidThreadId;
	 
	 // Whether task task observers are allowed.
	 bool allow_task_observers_ = true;

	 // An interface back to RunLoop state accessible by this RunLoop::Delegate.
	 //RunLoop::Delegate::Client* run_loop_client_ = nullptr;




};

}

#endif // !BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
