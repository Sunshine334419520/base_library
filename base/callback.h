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


/*
class BASE_EXPORT Closure {
 public:
	 Closure(const std::function<void()> fun)
		 : fun_(fun) {}

	 

	 explicit Closure(std::function<void()>&& fun) noexcept
		 : fun_(std::move(fun)) {}

	 Closure(const Closure& other)
		 : fun_(other.fun_) {}

	 Closure(const Closure&& other) noexcept
		 : fun_(std::move(other.fun_)) {}

	 Closure() : fun_(nullptr) {}

	 Closure& operator=(const Closure& other) = default;
	 Closure& operator=(Closure&& other) = default;

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
	explicit OnceClosure(std::function<void()> fun)
		: fun_(fun) {}
	explicit OnceClosure(std::function<void()>&& fun)
		: fun_(std::move(fun_)) {}


	OnceClosure(const OnceClosure& other) = delete;
	OnceClosure& operator=(const OnceClosure& other) = delete;

	OnceClosure(OnceClosure&& other) = default;
	OnceClosure& operator=(OnceClosure&& other) = default;

	~OnceClosure() = default;

	bool is_null() const { return fun_ == nullptr; }

	void Run() { 
		OnceClosure cb = std::move(*this);
		cb.fun_();
	}

	void Reset() { fun_ = nullptr; }

	explicit operator bool() { return !is_null(); }
 private:
	 std::function<void()> fun_;
};
*/




template <typename Fty>
using Callback = std::function<Fty>;


//using Closure = Callback<void()>;
//using Closure = Callback<void()>;
using Closure = std::function<void()>;

}

#endif // !BASE_CALLBACK_H
