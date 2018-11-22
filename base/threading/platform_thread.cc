#include "platform_thread.h"
/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: platform_thread.h
* @Last modified by:  YangGuang
*/

namespace base {

namespace {

struct ThreadParams {
	ThreadParams()
		: delegate(nullptr),
		  joinable(false),
		  priority(ThreadPriority::NORMAL) {}
		
	PlatformThread::Delegate* delegate;
	bool joinable;
	ThreadPriority priority;
};



void* ThreadFunc(void* params) {
	PlatformThread::Delegate* delegate = nullptr;

	{
		std::unique_ptr<ThreadParams> thread_params(
			static_cast<ThreadParams*>(params));

		delegate = thread_params->delegate;
		
		
		//if (!thread_params->joinable)
			// do somethink.
	}
	
	delegate->ThreadMain();

	return nullptr;
}

std::thread CreateThread(size_t stack_size,
				  bool joinable,
				  PlatformThread::Delegate* delegate,
				  ThreadPriority priority) {
	std::unique_ptr<ThreadParams> params(new ThreadParams);
	params->delegate = delegate;
	params->joinable = joinable;
	params->priority = priority;

	std::thread th(&ThreadFunc, params.get());
	
	if (!params->joinable)
		th.detach();
	params.release();
	return th;
}

}		// namespace 

PlatformThreadId PlatformThread::CurrentId() {
	return std::this_thread::get_id();
}

PlatformThreadHandle PlatformThread::CurrentHandle()
{
	return PlatformThreadHandle();
}

void PlatformThread::YieldCurrentThread() {
	std::this_thread::yield();
}

void PlatformThread::SetName(const std::string & name)
{
}

const char * PlatformThread::GetName()
{
	return nullptr;
}

std::thread 
PlatformThread::CreateWithPriority(size_t stack_size, 
											   Delegate * delegate,
											   ThreadPriority priority) {
	return std::move(CreateThread(stack_size, true,
					 delegate, priority));
}

std::thread 
PlatformThread::CreateNonJoinable(size_t stack_size,
											  Delegate * delegate) {
	return std::move(CreateThread(stack_size, false,
					 delegate, ThreadPriority::NORMAL));
}

std::thread 
PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
											  Delegate * delegate,
											  ThreadPriority priority) {
	return std::move(CreateThread(stack_size, false,
					 delegate, priority));
}

std::thread PlatformThread::Join(std::thread && thread) {
	if (thread.joinable())
		thread.join();
	return std::move(thread);
}

std::thread PlatformThread::Detch(std::thread && thread) {
	thread.detach();
	return std::move(thread);
}

bool PlatformThread::CanIncreaseCurrentThreadPriority() {
	return false;
}

void PlatformThread::SetThreadPriority(PlatformThreadHandle handle,
									   ThreadPriority priority)
{
}

ThreadPriority PlatformThread::GetThreadPriority()
{
	return ThreadPriority();
}

}