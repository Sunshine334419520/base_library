/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: lazy_instance_helpers.cc
* @Last modified by:  YangGuang
*/
#include "lazy_instance_helpers.h"

#include <chrono>
#include <thread>

#include "base/at_exit.h"
#include "base/logging.h"


namespace base {
namespace internal {

void* kLazyInstanceStateCreating(new int(1));
void* kLazyDefaultInstanceState = nullptr;

bool NeedsLazyInstance(std::atomic<void*>* state) {
	// 尝试去创建一个Instance,如果我们是是第一个到来的线程,返回true.
	// 否则代表不是第一个线程到来，可能产生线程对数据对争抢.
   if (state->compare_exchange_strong(kLazyDefaultInstanceState,
                                      kLazyInstanceStateCreating,
                                      std::memory_order_acq_rel)) {
	   // 是第一个线程到来，比较完之后，把state值原先对值复制为kLazyInstanceStateCreateing.
	   // 以为下一个到来对线程比较会失败, 从而可以达到同步对效果，可以直接返回true.
      return true;
   }

   // 不是第一个线程，在这里要保证数据对正确性，因此一直等待state不等于kLazyInstanceStateCreating.
   if (state->load(std::memory_order_acquire) == kLazyInstanceStateCreating) {
	   // 这种情况需要保护多线程的安全，让state != kLazyInstanceStateCreatingd，
	   // 当这里面不等于时，说明另一个线程修改了它的值，这个值现在是正确的，可以进行返回
      auto start = std::chrono::system_clock::now();

      do {
         const std::chrono::milliseconds elapsed = std::chrono::duration_cast<
                 std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

         const std::chrono::milliseconds max_sleep_time(1);

         if (elapsed < max_sleep_time)
            std::this_thread::yield();
         else
            std::this_thread::sleep_for(max_sleep_time);
      } while(state->load(std::memory_order_acquire)
              == kLazyInstanceStateCreating);
   }

   return false;
}

void CompleteLazyInstance(void (*destructor)(void*),
                          void* destructor_arg) {

	if (!destructor)
		AtExitManager::RegisterCallback(destructor, destructor_arg);
}


}   // namespace internal
}   // namespace bas