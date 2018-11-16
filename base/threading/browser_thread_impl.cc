/**
* @Author: YangGuang
* @Date:   2018-11-16
* @Email:  guang334419520@126.com
* @Filename: browser_thread.h
* @Last modified by:  YangGuang
*/
#include "base/threading/browser_thread.h"

namespace sun {

// static method.
bool BrowserThread::PostTask(ID identifier,
							 const base::Location& from_here,
							 base::OnceClosure task) {

}

// static method.
bool BrowserThread::PostDelayedTask(ID identifier,
									const base::Location & from_here,
									base::OnceClosure task,
									std::chrono::microseconds delay) {
	return false;
}

// static method.
bool BrowserThread::PostNonNestableTask(ID identifier,
										const base::Location & from_here,
										base::OnceClosure task) {
	return false;
}

// static method.
bool BrowserThread::PostNonNestableDelayedTask(ID identifier, const base::Location & from_here, base::OnceClosure task, std::chrono::milliseconds delay)
{
	return false;
}

// static method.
bool BrowserThread::PostTaskAndReply(ID identifier, const base::Location & from_here, base::OnceClosure task, base::OnceClosure reply)
{
	return false;
}

// static method.
void BrowserThread::PostAfterStartupTask(const base::Location & from_here, const std::shared_ptr<base::TaskRunner>& task_runner, base::OnceClosure task)
{
}

// static method.
std::shared_ptr<base::SingleThreadTaskRunner> BrowserThread::GetTaskRunnerForThread(ID identifier)
{
	return std::shared_ptr<base::SingleThreadTaskRunner>();
}

}	// namespace sun.