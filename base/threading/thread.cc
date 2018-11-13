#include "base/threading/thread.h"

#include <cassert>
#include <thread>

#include "base/logging.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/run_loop.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/lazy_instance.h"

namespace base {

namespace {

thread_local base::LazyInstance<bool>::Leaky lazy_tls_bool =
	LAZY_INSTANCE_INITIALIZER;
}	// namespace .

Thread::Options::Options() = default;

Thread::Options::Options(const Options & other) = default;

Thread::Options::~Options() = default;


Thread::Thread(const std::string & name)
	: name_(name){
	
}

Thread::~Thread() {
	Stop();
}

bool Thread::Start() {
	Options options;

	return StartWithOptions(options);
}

bool Thread::StartWithOptions(const Options & options) {

	DCHECK(!message_loop_);
	DCHECK(!IsRunning());
	DCHECK(!stopping_);
	DCHECK(!is_thread_valid_);

	id_ = kInvalidThreadId;

	//SetThreadWasQuitProperly(false);
	/*MessageLoop::Type type = options.message_loop_type;
	if (!options.message_pump_factory.is_null())
		type = MessageLoop::TYPE_CUSTOM;

	message_loop_timer_slack_ = options.timer_slack;
	std::unique_ptr<MessageLoop> message_loop_owned =
		MessageLoop::CreateUnbound(type, options.message_pump_factory);
	message_loop_ = message_loop_owned.get();*/

	{
		std::lock_guard<std::mutex> lock(thread_mutex_);
		thread_ = options.joinable
			? PlatformThread::CreateWithPriority(options.stack_size,
												 this, options.priority)
			: PlatformThread::CreateNonJoinableWithPriority(
				options.stack_size, this, options.priority);
		is_thread_valid_ = true;
	}

	joinable_ = options.joinable;
	
	// ignore_result(message_loop_owned_.release());

	DCHECK(message_loop_);
	return true;
}

bool Thread::WaitUntilThreadStarted() const {
	if (!message_loop_)
		return false;
	std::unique_lock<std::mutex> lock(cond_var_mutex_);
	start_event_.wait(lock);
	return true;
}

void Thread::Stop() {
	DCHECK(joinable_);

	std::lock_guard<std::mutex> lock(thread_mutex_);

	StopSoon();

	if (!thread_.joinable())
		return;

	PlatformThread::Join(std::move(thread_));

	is_thread_valid_ = false;

	DCHECK(!message_loop_);
	
	stopping_ = false;
}

void Thread::StopSoon() {
	if (stopping_ || !message_loop_)
		return;

	stopping_ = true;

	if (using_external_message_loop_) {
		DCHECK(!IsRunning());
		message_loop_ = nullptr;
		return;
	}

	task_runner()->PostTask(FROM_HERE, std::bind(&Thread::ThreadQuitHelper, this));
}

PlatformThreadId Thread::GetThreadId() const {
	std::unique_lock<std::mutex> lock(cond_var_mutex_);
	id_event_.wait(lock);
	return id_;
}

PlatformThreadHandle Thread::GetThreadHandle() {
	std::lock_guard<std::mutex> lock(thread_mutex_);
	
	return is_thread_valid_ ? thread_.native_handle() : nullptr;
}

bool Thread::IsRunning() const {
	if (message_loop_ && !stopping_)
		return true;

	std::lock_guard<std::mutex> lock(running_mutex_);
	return running_;
}

void Thread::Run(RunLoop * run_loop) {
	DCHECK(id_ == PlatformThread::CurrentId());
	DCHECK(is_thread_valid_);

	run_loop_->Run();
}

// static.
void Thread::SetThreadWasQuitProperly(bool flag) {
	auto tmp = lazy_tls_bool.Get();
	tmp = flag;
}

// static.
bool Thread::GetThreadWasQuitProperly() {
	bool quit_properly = true;
#ifndef NDEBUG
	quit_properly = lazy_tls_bool.Get();
#endif
	return quit_properly;
}

void Thread::SetMessageLoop(MessageLoop * message_loop) {
	DCHECK(message_loop);
	
	DCHECK(!IsRunning());
	message_loop_ = message_loop;
	DCHECK(IsRunning());

	using_external_message_loop_ = true;
}





void Thread::ThreadMain() {
	DCHECK(kInvalidThreadId == id_);
	id_ = PlatformThread::CurrentId();
	DCHECK(kInvalidThreadId != id_);
	id_event_.notify_one();

	PlatformThread::SetName(name_.c_str());

	DCHECK(message_loop_);
	std::unique_ptr<MessageLoop> message_loop(message_loop_);
	message_loop_->BindToCurrentThread();
	//message_loop_->SetTimerSlack(message_loop_timer_slack_);

	// Let the thread do extra initialization.
	Init();

	{
		std::lock_guard<std::mutex> lock(running_mutex_);
		running_ = true;
	}

	start_event_.notify_one();

	RunLoop run_loop;
	run_loop_ = &run_loop;
	Run(run_loop_);

	{
		std::lock_guard<std::mutex> lock(running_mutex_);
		running_ = false;
	}

	// Let the thread do extra cleanup.
	CleanUp();

	message_loop_ = nullptr;
	run_loop_ = nullptr;
}

void Thread::ThreadQuitHelper() {
	DCHECK(run_loop_);
	run_loop_->QuitWhenIdle();
}

}	// namespace 