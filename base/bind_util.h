/**
* @Author: YangGuang
* @Date:   2018-11-16
* @Email:  guang334419520@126.com
* @Filename: callback.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_BIND_UTIL_H
#define BASE_BIND_UTIL_H

#include <utility>
#include <functional>

namespace base {

template <typename Function, typename... Args>
inline Closure BindClosure(Function&& functor, Args&&... args) {
	return Closure(std::bind(std::forward<Function>(functor),
				   std::forward<Args>(args)...));
}

template <typename Function, typename... Args>
inline OnceClosure BindOnceClosure(Function&& functor, Args&&... args) {
	return OnceClosure(std::bind(std::forward<Function>(functor),
					   std::forward<Args>(args)...));
}

namespace internal {

// 这个是一个帮助函数，为了帮助bind绑定参数时，让绑定的参数函数运行完成之后就自动释放.
template <typename T>
class OwnedWrapper {
 public:
	 explicit OwnedWrapper(T* o) : ptr_(o) {}
	 ~OwnedWrapper() { delete ptr_; }
	 T* get() const { return ptr_; }
	 OwnedWrapper(OwnedWrapper&& other) noexcept {
		 ptr_ = other.ptr_;
		 other.ptr_ = nullptr;
	 }
 private:
	 mutable T* ptr_;
};


}

template <typename T>
inline internal::OwnedWrapper<T> Owned(T* o) {
	return internal::OwnedWrapper<T>(o);
}

}	// namespace base.

#endif // !BASE_BIND_UTIL_H