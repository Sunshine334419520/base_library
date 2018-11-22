/**
* @Author: YangGuang
* @Date:   2018-11-22
* @Email:  guang334419520@126.com
* @Filename: browser_process_sub_thread.cc
* @Last modified by:  YangGuang
*/
#include "base/threading/browser_process_sub_thread.h"

#include "base/threading/browser_thread_impl.h"
#include "base/bind_util.h"
namespace sun {

BrowserProcessSubThread::BrowserProcessSubThread(BrowserThread::ID identifier)
	: base::Thread(BrowserThreadImpl::GetThreadName(identifier)),
	  identifier_(identifier) {
}

BrowserProcessSubThread::~BrowserProcessSubThread() {
	Stop();
}

void BrowserProcessSubThread::RegisterAsBrowserThread() {
	DCHECK(IsRunning());

	DCHECK(!browser_thread_);
	browser_thread_.reset(new BrowserThreadImpl(identifier_, task_runner()));

	task_runner()->PostTask(FROM_HERE,
		base::BindOnceClosure(
		&BrowserProcessSubThread::CompleteInitializationOnBrowserThread,
		this));
}

std::unique_ptr<BrowserProcessSubThread>
BrowserProcessSubThread::CreateIOThread()
{
	return std::unique_ptr<BrowserProcessSubThread>();
}

void BrowserProcessSubThread::Init()
{
}

void BrowserProcessSubThread::Run(base::RunLoop * run_loop) {
	switch (identifier_) {
	case BrowserThread::UI:
		UIThreadRun(run_loop);
		break;
	case BrowserThread::IO:
		IOThreadRun(run_loop);
		break;
	case BrowserThread::ID_COUNT:
		break;
	}
}

void BrowserProcessSubThread::CleanUp()
{
}

void BrowserProcessSubThread::CompleteInitializationOnBrowserThread()
{
}

void BrowserProcessSubThread::UIThreadRun(base::RunLoop * run_loop)
{
	Thread::Run(run_loop);

}

void BrowserProcessSubThread::IOThreadRun(base::RunLoop * run_loop)
{
	Thread::Run(run_loop);
}

void BrowserProcessSubThread::IOThreadCleanUp()
{
}

}