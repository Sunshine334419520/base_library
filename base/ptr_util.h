/**
* @Author: YangGuang
* @Date:   2018-10-29
* @Email:  guang334419520@126.com
* @Filename: ptr_util.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_PTR_UTIL_H
#define BASE_PTR_UTIL_H

#include <utility>
#include <memory>

#include "base/default_destroy.h"

namespace base {

// 帮助函数，为了帮助shared_ptr 创建对象，并且拥有一个默认的删除函数.
template <typename T,
		  typename Traits = DefaultDestroyTraits>
std::shared_ptr<T> WrapShared(T* ptr) {
	return std::shared_ptr<T>(ptr, &Traits::Destroy<T>);
}

template <typename T>
std::unique_ptr<T> WrapUnique(T* ptr) {
	return std::unique_ptr<T>(ptr);
}

template <typename T,
	typename Traits = DefaultDestroyTraits>
	std::shared_ptr<T> WrapSharedWithMakeShared(T* ptr) {
	return std::make_shared<T>(ptr, &Traits::Destroy<T>);
}

}	// namespace base.

#endif // !BASE_PTR_UTIL_H
