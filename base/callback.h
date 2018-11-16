/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: callback.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_CALLBACK_H
#define BASE_CALLBACK_H

#include <functional>
#include <memory>

#include "base/base_export.h"

namespace base {

class OnceClosure;

class BASE_EXPORT Closure {
 public:
	 Closure(const std::function<void()> fun)
		 : fun_(fun) {}
	 
	 Closure(const Closure& other)
		 : fun_(other.fun_) {}

	 Closure(const Closure&& other) noexcept
		 : fun_(std::move(other.fun_)) {}

	 Closure() : fun_(nullptr) {}

	 Closure& operator=(const Closure& other) = default;
	 Closure& operator=(Closure&& other) = default;

	 Closure& operator=(std::function<void()> fun) {
		 this->fun_ = fun;
		 return *this;
	 }

	/* Closure& operator=(std::function<void()>&& fun) {
		 fun_ = fun;
		 fun = nullptr;
		 return 
	 }*/

	 ~Closure() = default;

	 bool is_null() const { return fun_ == nullptr; }

	 void Run() { fun_(); }

	 void Reset() { fun_ = nullptr; }

	 explicit operator bool() { return !is_null(); }

	 template <typename Fty>
	 operator std::function<Fty>() { return fun_; }

 private:
	 std::function<void()> fun_;
};

class BASE_EXPORT OnceClosure {
 public:
	OnceClosure() : fun_(nullptr) {}

	OnceClosure(std::function<void()>&& fun)
		: fun_(std::move(fun)) {}

	OnceClosure(Closure&& other)
		: fun_(std::move(other)) {}
		


	OnceClosure(const OnceClosure& other) = delete;
	OnceClosure& operator=(const OnceClosure& other) = delete;

	OnceClosure(OnceClosure&& other) = default;
	OnceClosure& operator=(OnceClosure&& other) = default;

	~OnceClosure() = default;

	bool is_null() const { return fun_ == nullptr; }

	void Run() & { 
		OnceClosure cb = std::move(*this);
		cb.fun_();
	}

	void Run() && {
		OnceClosure cb = std::move(*this);
		std::move(cb).fun_();
	}

	void Reset() { fun_ = nullptr; }

	explicit operator bool() { return !is_null(); }
	template <typename Fty>
	operator std::function<Fty>() { return fun_; }
 private:
	 std::function<void()> fun_;
};




template <typename Fty>
using Callback = std::function<Fty>;




}

#endif // !BASE_CALLBACK_H
