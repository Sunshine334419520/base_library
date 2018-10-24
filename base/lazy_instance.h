/**
* @Author: YangGuang
* @Date:   2018-10-20
* @Email:  guang334419520@126.com
* @Filename: lazy_instance.h
* @Last modified by:  YangGuang
*/

// 这是一个对创建单实例类的一个封装.
// Example usage:
//   static LazyInstance<MyClass>::Leaky inst = LAZY_INSTANCE_INITIALIZER;
//   void SomeMethod() {
//     inst.Get().SomeMethod();  // MyClass::SomeMethod()
//
//     MyClass* ptr = inst.Pointer();
//     ptr->DoDoDo();  // MyClass::DoDoDo
//   }

#ifndef BASE_LAZY_INSTANCE_H
#define BASE_LAZY_INSTANCE_H

#include <new>	// For placement new.
#include <atomic>


#include "base/base_export.h"
#include "base/lazy_instance_helpers.h"
#include "macor.h"

#define LAZY_INSTANCE_INITIALIZER {nullptr}

namespace base {

template <typename Type>
struct LazyInstanceTraitsBase {
	static Type* New(void* instance) {
		// 调用instance 的构造函数进行初始化. 并不是分配内存
		// 使用的是placement new. 而不是C++ 的new
		return new (instance) Type();
	}

	static void CallDestructor(Type* intance) {
		// Call the destructor.
		intance->~Type();
	}
};

namespace internal {

// 这个traits class会在程序终止后在AtExitManager上面析构这个类,
// 这个可能不是你想要的，用下面的Leaky.
template <typename Type>
struct DestructorAtExitLazyInstanceTraits {
	static const bool kRegisterOnExit = true;

	static Type* New(void* instance) {
		return LazyInstanceTraitsBase<Type>::New(instance);
	}

	static void Delete(Type* instance) {
		LazyInstanceTraitsBase<Type>::CallDestructor(instance);
	}
};

template <typename Type>
struct LeakyLazyInstanceTraits {
	static const bool kRegisterOnExit = false;

	static Type* New(void* instance) {
		return LazyInstanceTraitsBase<Type>::New(instance);
	}

	static void Delete(Type* instance) {

	}
};

template <typename Type>
struct ErrorMustSelectLazyOrDestructorAtExitForLazyInstance {};

}	// namespace internal.

template <typename Type,
		  typename Traits = 
			internal::ErrorMustSelectLazyOrDestructorAtExitForLazyInstance<Type>>
class LazyInstance {
 public:	
	 typedef LazyInstance<Type, internal::LeakyLazyInstanceTraits<Type>> Leaky;
	 typedef LazyInstance<Type, internal::DestructorAtExitLazyInstanceTraits<Type>>
		 DestructorAtExit;

	 Type& Get() {
		 return *Pointer();
	 }

	 Type* Pointer() {
		 return subtle::GetOrCreateLazyPointer(&private_instance_, &Traits::New, private_buf_,
											   Traits::kRegisterOnExit ? OnExit : nullptr, this);
	 }

	 bool IsCreated() {
		 return private_instance_ != nullptr;
	 }

#if defined(OS_WIN)
#pragma warning(push)
#pragma warning(disable: 4324)
#endif
	 std::atomic<Type*> private_instance_;
	 alignas(Type) char private_buf_[sizeof(Type)];

#if defined(OS_WIN)
#pragma warning(pop)
#endif

 private:
	 Type* instance() {
		 return private_instance_.load(std::memory_order_relaxed);
	 }

	 static void OnExit(void* lazy_instance) {
		 LazyInstance<Type, Traits>* me =
			 reinterpret_cast<LazyInstance<Type, Traits>*>(lazy_instance);
		 Traits::Delete(me->instance());
		 me->private_instance_.store(nullptr);
		 
	 }

};

}

#endif // !BASE_LAZY_INSTANCE_H

