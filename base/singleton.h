/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: singleton.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_SIGNALETON_H
#define BASE_SIGNALETON_H

#include <atomic>

#include "base/base_export.h"
#include "base/lazy_instance_helpers.h"



namespace base {

template <typename Type>
struct DefaultSingletonTraits {
    // Allocates the object.
    static Type* New() {
        return new Type();
    }

    // Destroys the object.
    static void Delete(Type* x) {
        delete x;
    }

    static const bool kRegisterAtExit = true;

};

template <typename Type>
struct LeakySingletonTraits : public DefaultSingletonTraits<Type> {
	static const bool kRegisterAtExit = false;
};


// for example.
// Singleton<Type>::get();
template <typename Type,
          typename Traits = DefaultSingletonTraits<Type>,
           typename DifferentiatingType = Type>
class Singleton {
 private:
	 // 要想使用这个类，Type必须包含GetInstance函数，并且遵循制定对规则.
    friend Type* Type::GetInstance();

	// 获得一个Type类型指针，外部使用这个函数获得一个线程安全的单实例指针.
    static Type* get() {
        return subtle::GetOrCreateLazyPointer<Type>(&instance_, CreatorFunc,
													nullptr, Traits::kRegisterAtExit
													? OnExit : nullptr, nullptr);
    }

    static Type* CreatorFunc(void* /* creator_arg*/) { return Traits::New(); }

	// 删除函数并不需要保证线程安全性，它仅仅在进程终止前调用.
	// 这个函数只会在一个线程调用. 所以使用std::memory_order_relaxed.
    static void OnExit(void* /* unused*/) {
        Traits::Delete(instance_.load(std::memory_order_relaxed));
        instance_.store(reinterpret_cast<Type*>(internal::kLazyDefaultInstanceState), 
						 std::memory_order_relaxed);
    }

    static std::atomic<Type*> instance_;    
};

template <typename Type, typename Traits, typename DifferentiatingType>
std::atomic<Type*> Singleton<Type, Traits, DifferentiatingType>::instance_ =
				reinterpret_cast<Type*>(base::internal::kLazyDefaultInstanceState);


}



#endif // BASE_SIGNALETON_H