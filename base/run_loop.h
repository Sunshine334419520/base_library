/**
* @Author: YangGuang
* @Date:   2018-10-29
* @Email:  guang334419520@126.com
* @Filename: run_loop.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_RUN_LOOP_H
#define BASE_RUN_LOOP_H

#include <utility>
#include <vector>
#include <stack>
#include <memory>
#include <list>

#include "base/base_export.h"
#include "base/callback.h"
#include "macor.h"

namespace base {

class SingleThreadTaskRunner;

// 一个帮助类，帮助RunLoop::Deleagte 和当前线程进行关联.
// 一个RunLoop::Delegate在还没有绑定到一个线程上( 
// 用RunLoop::RegisterDelegateForCurrentThread()) 时，调用所有的RunLoop
// 方法都时不安全的，包括static method，除非有显示表明， RunLoop::Run 只能够
// 在RunLoop生命周期内调用一次， 在stack上创建一个RunLoop对象，并且调用Run/Quit
// 在nested RunLoop， 但是不要使用nested loops 在生产代码上.
class BASE_EXPORT RunLoop {

 public:
	// 顶层的kDefault RunLoop non-nested会处理绑定到它自己的系统和应用级别的任务,
	// 当处理一个嵌套时，kDefault RunLoop将只会处理系统任务，而kNestableTasksAllowed
	// RunLoop将会处理应用级别的任务.
	//
	// 在默认的情况下，嵌套任务将会被禁止.
	// 在通常情况下，需要避免使用nestable RunLoops, 想要正确使用它是危险和困难的,
	// 所以请尽量避免使用.
	enum class Type {
		kDefault,
		kNestableTasksAllowed,
	};

	explicit RunLoop(Type type = Type::kDefault);
	~RunLoop();

	// 运行当前的RunLoop::Delegate, 一直到调用Quit时才会退出.
	void Run();

	// 运行当前的RunLoop::Delegate直到queue上找不到任务的消息或者任务,
	// WARNING: 这个可能永远都不会返回
	void RunUntilIdle();

	bool running() const {
		//return running_;
	}
	
	// Quit() 是立即退出一个在先前运行的RunLoop, QuitWhenIdle() 是在队列上没有
	// 任务或者消息时退出一个先前运行的RunLoop.
	// 
	// 这些方法都是线程安全的，但是最好还是在不同的线程上调用Quit().
	// Quit() and QuitWhenIdle() 都可以调用在Run()的前面，或者运行的时候，
	// 或者是Run()已经结束了, 在Run结束了之后再调用Quit() or QuitWhenIdle()是
	// 不会有影响的.
	//
	// WARNING: 你必须确保调用Quit() or QuitWhenIdle() 来终止RunLoop和message loop,
	// 如果你不调用，我们又启动了一个nestable RunLoop ，那样就会导致message loop永远都
	// 不会退出.
	void Quit();
	void QuitWhenIdle();
	
	// Example:
	//	RunLoop run_loop;
	// PostTask(run_loop.QuitClosure());
	// run_loop.Run();
	Closure QuitClosure();
	Closure QuitWhenIdleClosure();

	// 如果又一个RunLoop活跃在当前线程，那么就返回true, 这个是安全的在还没调用
	// RegisterDelegateForCurrentThread(). 之后也是安全的.
	static bool IsRunningOnCurrentThread();

	// 如果有一个RunLoop活跃在当前的线程，并且这个RunLoop还是在另一个RunLoop里面，
	// 那样就返回true.
	// 这个是安全的在还没调用ReisterDelegateForCurrentThread(), 之后也是安全的.
	static bool IsNestedOnCurrentThread();
	
	// 通知在一个nested runloop 运行前和后.
	class BASE_EXPORT NestingObserver {
	 public:
		 // 通知再当前线程开始运行任务工作之前.
		 virtual void OnBeginNestedRunLoop() = 0;
		 // 通知再当前线程完成运行任务之后.
		 virtual void OnExitNestedRunLoop() {}

	 protected:
		 virtual ~NestingObserver() = default;

	};

	static void AddNestingObserverOnCurrentThread(
		std::shared_ptr<NestingObserver> observer);
	static void RemoveNestingObserverOnCurrentThread(
		std::shared_ptr<NestingObserver> observer);

	class BASE_EXPORT Delegate {
	 public:
		 Delegate();
		 virtual ~Delegate();

		 // RunLoop的Run和Quit会调用这个类的Run和Quit.
		 virtual void Run(bool application_tasks_allowed) = 0;
		 virtual void Quit() = 0;

		 // Invoked right before a RunLoop enters a nested Run() call on this
		 // Delegate iff this RunLoop is of type kNestableTasksAllowed. The Delegate
		 // should ensure that the upcoming Run() call will result in processing
		 // application tasks queued ahead of it without further probing. e.g.
		 // message pumps on some platforms, like Mac, need an explicit request to
		 // process application tasks when nested, otherwise they'll only wait for
		 // system messages.
		 virtual void EnsureWorkScheduled() = 0;
	 protected:
		 // 这个函数应该旨在Delegate里面调用，返回这个Delegate's 的
		 // |should_quit_when_idle_callback|。
		 bool ShouldQuitWhenIdle();

	 private:
		 // 下面这些只会再RunLoop 中使用.
		 friend class RunLoop;

		 // 一个用vector来当底层实现的stack比用默认的deque当底层内存效率要高一些，
		 // 对于我们这个RunLoop来说.
		 using RunLoopStack = std::stack<RunLoop*, std::vector<RunLoop*>>;

		 RunLoopStack active_run_loops_;
		 std::list<std::shared_ptr<RunLoop::NestingObserver>> nesting_observers_;

		 // 如果说是绑定到某个线程就为true.
		 bool bound_ = false;

		 DISALLOW_COPY_AND_ASSIGN(Delegate);
	};

	// 在使用RunLoop方法之前必须要调用这个函数来再绑定delegate到当前线程.
	static void RegisterDelegateForCurrentThread(Delegate* delegate);

	// 退出活动运行循环(空闲时)——必须有一个。这些被作为优先的临时替换引入，
	// 以替代长期废弃的MessageLoop::Quit(idle)(Closure)方法。调用者应
	// 该正确地查找对适当的RunLoop实例(或它的QuitClosure)的引用，而不是
	// 使用这些引用，以便将Run()/Quit()链接到单个RunLoop实例并提高可读性。
	static void QuitCurrentDeprecated();
	static void QuitCurrentWhenIdleDeprecated();
	static OnceClosure QuitCurrentWhenIdleClosureDeprecated();


 private:
	 // Return false to abort the Run.
	 bool BeforeRun();
	 void AfterRun();

	 Delegate* delegate_;

	 const Type type_;

	 bool quit_called_ = false;
	 bool running_ = false;

	 // 这个是用来记录调用了QuitWhenIdle再这个消息循环上，意味着这个Delegate
	 // 应该再闲置的时候退出.
	 bool quit_when_idle_received_ = false;

	 // 如果允许使用QuitCurrent*Deprecated()，则为True。从RunLoop中使用
	 // Quit*Closure()隐式地将其设置为false，因此在运行RunLoop()时不能使用
	 // QuitCurrent*Deprecated()。
	 bool allow_quit_current_deprecated_ = true;

	 const std::shared_ptr<SingleThreadTaskRunner> origin_task_runner_;

	 // std::weak_ptr 来保证安全的删除,用来防止，外部使用std::shared_ptr
	 // 对RunLoop.
	 std::weak_ptr<RunLoop> weak_factory_;

	 DISALLOW_COPY_AND_ASSIGN(RunLoop);
};

}	// namespace base.

#endif // !BASE_RUN_LOOP_H
