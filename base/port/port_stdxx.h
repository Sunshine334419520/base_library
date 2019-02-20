/**
* @Author: YangGuang
* @Date:   2019-02-14
* @Email:  guang334419520@126.com
* @Filename: port_stdxx.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_PORT_PORT_STDXX_H
#define BASE_PORT_PORT_STDXX_H

#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <condition_variable>  // NOLINT
#include <mutex>               // NOLINT
#include <string>

#include "base/port/thread_annotations.h"

class CondVar;

// std::mutex 的一个非常小的wrap
class Mutex {
 public:
     Mutex() = default;
     ~Mutex() = default;

     Mutex(const Mutex&) = delete;
     Mutex operator=(const Mutex&) = delete;

     void Lock() EXCLUSIVE_LOCK_FUNCTION() { mu_.lock(); }
     void Unlock() UNLOCK_FUNCTION() { mu_.unlock(); }
     void AssertHeld() ASSERT_EXCLUSIVE_LOCK() {}
 private:
     friend class CondVar;
     std::mutex mu_;
};

// std::condition_variable的一个简单wrap
class CondVar {
 public:
     explicit CondVar(Mutex* mutex) : mu_(mutex) { assert(mu_ != nullptr); }
     ~CondVar() = default;

     CondVar(const CondVar&) = delete;
     CondVar operator=(const CondVar&) = delete;

     void Wait() {
         std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
         cv_.wait(lock);
         lock.release();
     }
     void Signal() { cv_.notify_one(); }
     void SignalAll() { cv_.notify_all(); }
 private:
     std::condition_variable cv_;
     Mutex* const mu_;
};
#endif // !BASE_PORT_PORT_STDXX_H
