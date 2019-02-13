/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: thread_pool.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_THREADING_THREAD_POOL_H
#define BASE_THREADING_THREAD_POOL_H

#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>


#include "base/base_export.h"
#include "base/thread_safe_queue.h"
#include "base/task/function_wrapper.h"
#include "base/work_stealing_queue.h"

namespace base {

class BASE_EXPORT ThreadPool {
 public:
	static ThreadPool* Current();

	explicit ThreadPool(std::size_t thread_num = 
						std::thread::hardware_concurrency());

	void Start();

	void RunPendingTask();

	void JoinAll();

	template <typename Function>
	auto AddWork(Function f)
		->std::future<typename std::result_of<Function()>::type>;

	~ThreadPool();

 private:
	 using Task = base::FunctionWrapper;
	 void WorkerThread(unsigned int index);
	 bool PopTaskFromLocalQueue(Task& task) {
		 return local_work_queue_ && local_work_queue_->TryPop(task);
	 }

	 bool PopTaskFromPoolQueue(Task& task) {
		 return pool_work_queue_.TryPop(task);
	 }

	 bool PopTaskFromOtherThreadQueue(Task& task) {
		 const std::size_t size = queues_.size();
		 // 从index后面的线程队列开始偷取任务.
		 for (unsigned int i = 0; i < size; ++i) {
			 const unsigned int index = (index_ + i + 1) % size;
			 if (queues_[index_]->TrySteal(task))
				 return true;
		 }
		 return false;
	 }

	 std::atomic_bool running_ = false;
	 base::ThreadSafeQueue<Task> pool_work_queue_;
	 std::vector<std::unique_ptr<internal::WorkStaealinggQueue>> queues_;
	 std::condition_variable cond_var_queues_;

	 std::size_t thread_num_;
	 std::vector<std::thread> threads_;
	 static thread_local internal::WorkStaealinggQueue* local_work_queue_;
	 static thread_local unsigned int index_;
};



template<typename Function>
inline auto ThreadPool::AddWork(Function f) 
	-> std::future<typename std::result_of<Function()>::type> {
	typedef typename std::result_of<Function()>::type result_type;
	std::packaged_task<result_type()> task(f);
	std::future<result_type> res(task.get_future());

	if (local_work_queue_)
		local_work_queue_->Push(std::move(task));
	else
		pool_work_queue_.Push(std::move(task));

	return res;
}

template <typename Fun, typename... Args>
inline auto PostTaskToThreadPool(Fun f, Args... args)
	->std::future<typename std::result_of<Fun(Args...)>::type> {
	return ThreadPool::Current()->AddWork(
		std::bind(std::forward<Fun>(f), std::forward<Args>(args)...));
}

}

#endif // !BASE_THREADING_THREAD_POOL_H
