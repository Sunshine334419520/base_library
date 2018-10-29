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
	// WARNING: 这个可能永远都不会放回,
	void RunUntilIdle();

	bool running() const {
		//return running_;
	}
	

};

}	// namespace base.

#endif // !BASE_RUN_LOOP_H
