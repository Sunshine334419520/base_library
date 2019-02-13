/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: thread_pool.h
* @Last modified by:  YangGuang
*/

#include "base/threading/thread_pool.h"

#include <exception>

//#include "base/logging.h"
#include <glog/logging.h>

namespace {

base::ThreadPool* g_instance = nullptr;

}

namespace base {

thread_local internal::WorkStaealinggQueue*
	ThreadPool::local_work_queue_ = nullptr;
thread_local unsigned int ThreadPool::index_ = 0;

ThreadPool * ThreadPool::Current() {
	return g_instance;
}

ThreadPool::ThreadPool(std::size_t thread_num)
	: thread_num_(thread_num){
	DCHECK(!g_instance);
	g_instance = this;
}

void ThreadPool::Start() {
	if (thread_num_ <= 0)
		throw std::logic_error("thread_num_ less 0");
	DCHECK(!running_);
	running_ = true;
	
	try {
		for (unsigned int i = 0; i < thread_num_; i++) {
			queues_.push_back(std::unique_ptr<internal::WorkStaealinggQueue>(
				new internal::WorkStaealinggQueue));
			threads_.push_back(
				std::thread(&ThreadPool::WorkerThread, this, i));
		}

		cond_var_queues_.notify_all();
	}
	catch (...) {
		cond_var_queues_.notify_all();
		running_ = false;
		throw std::runtime_error("Start failed");
	}
}

void ThreadPool::RunPendingTask() {
	Task task;
	// 有三种从队列中去任务的方式，首先会尝试从自己的队列中取队列,如果没有, 会尝试去
	// 共享的队列取数据，还是没有，就会从友军线程取偷取任务执行，友军线程也没有哪就是
	// 没有任务可执行，就休息一会.
	if (PopTaskFromLocalQueue(task) ||
		PopTaskFromPoolQueue(task) ||
		PopTaskFromOtherThreadQueue(task)) {
		task();
	}
	else {
		std::this_thread::yield();
	}
}

void ThreadPool::JoinAll() {
	DCHECK(running_);
	running_ = false;
	for (auto it = threads_.begin(); it != threads_.end(); ++it) {
		if (it->joinable()) {
			it->join();
		}
	}

	for (auto& queue : queues_) {
		queue.reset();
	}

	queues_.swap(std::vector<std::unique_ptr<internal::WorkStaealinggQueue>>());
	pool_work_queue_.Clear();
	threads_.swap(std::vector<std::thread>());
}

ThreadPool::~ThreadPool() {
	DCHECK(g_instance);
	if (running_) {
		running_ = false;
	}
	else if (!threads_.empty()){
		for (auto it = threads_.begin(); it != threads_.end(); ++it) {
			if (it->joinable()) {
				it->join();
			}
		}
	}
	
	g_instance = nullptr;
}

void ThreadPool::WorkerThread(unsigned int index) {
	// 等待Start函数完成
	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		cond_var_queues_.wait(lock);
	}
	

	index_ = index;
	local_work_queue_ = queues_[index_].get();

	while (running_) {
		RunPendingTask();
	}
}

}	// namespace base.
