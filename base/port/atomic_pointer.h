/**
* @Author: YangGuang
* @Date:   2019-02-14
* @Email:  guang334419520@126.com
* @Filename: atomic_pointer.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_PORT_ATOMIC_POINTER_H
#define BASE_PORT_ATOMIC_POINTER_H

#include <atomic>

namespace base {

class AtomicPointer {
 public:
     AtomicPointer() {}
     explicit AtomicPointer(void* ptr)
         : rep_(ptr) { }
     inline void* Acquire_Load() const {
         return rep_.load(std::memory_order_acquire);
     }
     inline void Release_Store(void* v) {
         return rep_.store(v, std::memory_order_release);
     }
     inline void* NoBarrier_Load() const {
         return rep_.load(std::memory_order_relaxed);
     }
     inline void NoBarrier_Store(void* v) {
         rep_.store(v, std::memory_order_relaxed);
     }

 private:
     std::atomic<void*> rep_;
};

}   // namespace base.

#endif // !BASE_PORT_ATOMIC_POINTER_H
