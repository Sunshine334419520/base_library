/**
* @Author: YangGuang
* @Date:   2018-11-16
* @Email:  guang334419520@126.com
* @Filename: browser_thread.h
* @Last modified by:  YangGuang
*/
#include "base/threading/browser_thread_impl.h"

#include <atomic>

#include "base/threading/browser_thread.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/task_runner_util.h"
#include "base/ptr_util.h"

namespace sun {

// 一个SingleThreadTaskRunner的实例化，专门针对于BrowserThread.
class BrowserThreadTaskRunner : public base::SingleThreadTaskRunner {
 public:
	explicit BrowserThreadTaskRunner(BrowserThread::ID identifier)
		: id_(identifier) {}

	bool PostDelayedTask(const base::Location& from_here,
						 base::OnceClosure task,
						 std::chrono::milliseconds delay) OVERRIDE {
		return BrowserThread::PostDelayedTask(id_, from_here, std::move(task),
											  delay);
	}

	bool PostNonNestableDelayedTask(const base::Location& from_here,
								   base::OnceClosure task,
								   std::chrono::milliseconds delay) OVERRIDE {
		return BrowserThread::PostNonNestableDelayedTask(id_, from_here,
														 std::move(task),
														 delay);
		
	}

	bool RunsTasksInCurrentSequence() OVERRIDE {
		return BrowserThread::CurrentlyOn(id_);
	}

 protected:
	 friend struct base::DefaultDestroyTraits;
	 ~BrowserThreadTaskRunner() OVERRIDE {}
 private:
	 BrowserThread::ID id_;
	 DISALLOW_COPY_AND_ASSIGN(BrowserThreadTaskRunner);
};

// 这个结构体是一个帮助BrowserThreadTaskRunner初始化的.
struct BrowserThreadTaskRunners {
	BrowserThreadTaskRunners() {
		for (int i = 0; i < BrowserThread::ID_COUNT; ++i) {
			/*proxies[i] =
				std::make_shared<BrowserThreadTaskRunner>(
					static_cast<BrowserThread::ID>(i));*/
			proxies[i] = 
				std::move(base::WrapShared(new BrowserThreadTaskRunner(
					static_cast<BrowserThread::ID>(i))));
		}
	}

	std::shared_ptr<base::SingleThreadTaskRunner> proxies[BrowserThread::ID_COUNT];
};

base::LazyInstance<BrowserThreadTaskRunners>::Leaky g_task_runners =
	LAZY_INSTANCE_INITIALIZER;

// 一个browser thread状态为了BrowserThread::ID.
enum BrowserThreadState {
	// BrowserThread::ID 还没有和任何的东西关联.
	UNINITIALIZED = 0,
	// BrowserThread::ID 已经和TaskRunner进行了关联，并且是又接受到任务
	RUNNING,
	// BrowserThread::ID 已经不会接受任务的tasks(它仍然还是和TaksRunner关联).
	SHTUDOWN
};

struct BrowserThreadGlobals {
	BrowserThreadGlobals() {}

	std::shared_ptr<base::SingleThreadTaskRunner>
		task_runners[BrowserThread::ID_COUNT];

	std::atomic<BrowserThreadState> states[BrowserThread::ID_COUNT] = {};
};

base::LazyInstance<BrowserThreadGlobals>::Leaky
	g_globals = LAZY_INSTANCE_INITIALIZER;

bool PostTaskHelper(BrowserThread::ID identifier,
					const base::Location& from_here,
					base::OnceClosure task,
					std::chrono::milliseconds delay,
					bool nestable) {
	DCHECK_GE(identifier, 0);
	DCHECK_LT(identifier, BrowserThread::ID_COUNT);

	BrowserThreadGlobals& globals = g_globals.Get();

	DCHECK_GE(globals.states[identifier].load(),
			  BrowserThreadState::RUNNING);
	DCHECK(globals.task_runners[identifier]);

	if (nestable) {
		return globals.task_runners[identifier]->PostDelayedTask(
			from_here, std::move(task), delay);
	}
	else {
		return globals.task_runners[identifier]->PostNonNestableDelayedTask(
			from_here, std::move(task), delay);
	}
}

BrowserThreadImpl::BrowserThreadImpl(
	BrowserThread::ID identifier,
	std::shared_ptr<base::SingleThreadTaskRunner> task_runner)
	: identifier_(identifier) {
	DCHECK_GE(identifier_, 0);
	DCHECK_LT(identifier_, ID_COUNT);
	DCHECK(task_runner);

	BrowserThreadGlobals& globals = g_globals.Get();

	DCHECK_EQ(globals.states[identifier_].load(std::memory_order_acquire),
			  BrowserThreadState::UNINITIALIZED);
	globals.states[identifier_].store(BrowserThreadState::RUNNING,
									  std::memory_order_seq_cst);

	DCHECK(!globals.task_runners[identifier_]);
	globals.task_runners[identifier] = std::move(task_runner);
}


BrowserThreadImpl::~BrowserThreadImpl() {
	BrowserThreadGlobals& globals = g_globals.Get();

	DCHECK_EQ(globals.states[identifier_].load(std::memory_order_acquire), 
			  BrowserThreadState::RUNNING);
	globals.states[identifier_].store(BrowserThreadState::SHTUDOWN,
									  std::memory_order_seq_cst);

	DCHECK(globals.task_runners[identifier_]);
}
const char * BrowserThreadImpl::GetThreadName(BrowserThread::ID thread) {
	static const char* const kBrowserThreadNames[BrowserThread::ID_COUNT] = {
		"",                 // UI (name assembled in browser_main_loop.cc).
		"Chrome_IOThread",  // IO
	};

	if (BrowserThread::UI < thread && thread < BrowserThread::ID_COUNT)
		return kBrowserThreadNames[thread];
	if (thread == BrowserThread::UI)
		return "Chrome_UIThread";
	return "Unknown Thread";
}

// static.
bool BrowserThread::IsThreadinitialized(ID identifier) {
	DCHECK_GE(identifier, 0);
	DCHECK_LT(identifier, ID_COUNT);

	BrowserThreadGlobals& globals = g_globals.Get();
	return globals.states[identifier].load(std::memory_order_acquire) ==
		   BrowserThreadState::RUNNING;
}

// static method.
bool BrowserThread::CurrentlyOn(ID identifier) {
	DCHECK_GE(identifier, 0);
	DCHECK_LT(identifier, ID_COUNT);

	BrowserThreadGlobals& globals = g_globals.Get();
	return globals.task_runners[identifier] &&
		   globals.task_runners[identifier]->RunsTasksInCurrentSequence();
}

bool BrowserThread::GetCurrentThreadIdentifier(ID* identifier) {
	BrowserThreadGlobals& globals = g_globals.Get();

	for (int i = 0; i < ID_COUNT; ++i) {
		if (globals.task_runners[i] &&
			globals.task_runners[i]->RunsTasksInCurrentSequence()) {
			*identifier = static_cast<ID>(i);
			return true;
		}
	}
	return false;
}

// static method.
bool BrowserThread::PostTask(ID identifier,
							 const base::Location& from_here,
							 base::OnceClosure task) {
	return PostTaskHelper(identifier, from_here, std::move(task),
						  std::chrono::milliseconds(0), true);
}

// static method.
bool BrowserThread::PostDelayedTask(ID identifier,
									const base::Location & from_here,
									base::OnceClosure task,
									std::chrono::milliseconds delay) {
	return PostTaskHelper(identifier, from_here, std::move(task),
						  delay, true);
}

// static method.
bool BrowserThread::PostNonNestableTask(ID identifier,
										const base::Location & from_here,
										base::OnceClosure task) {
	return PostTaskHelper(identifier, from_here, std::move(task),
						  std::chrono::milliseconds(0), false);
}

// static method.
bool BrowserThread::PostNonNestableDelayedTask(ID identifier,
											   const base::Location & from_here,
											   base::OnceClosure task,
											   std::chrono::milliseconds delay) {
	return PostTaskHelper(identifier, from_here, std::move(task),
						  delay, false);
}

// static method.
bool BrowserThread::PostTaskAndReply(ID identifier,
									 const base::Location & from_here,
									 base::OnceClosure task,
									 base::OnceClosure reply) {
	return GetTaskRunnerForThread(identifier)
		->PostTaskAndReplay(from_here, std::move(task), std::move(reply));
}

// static method.
void BrowserThread::PostAfterStartupTask(const base::Location & from_here,
										 const std::shared_ptr<base::TaskRunner>& task_runner,
										 base::OnceClosure task) {
}

// static method.
std::shared_ptr<base::SingleThreadTaskRunner> 
BrowserThread::GetTaskRunnerForThread(ID identifier) {
	return g_task_runners.Get().proxies[identifier];
}

}	// namespace sun.