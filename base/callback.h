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

namespace base {

template <typename R, typename... Args>
class Callback {//: public std::function<R(Args...> {
 public:
	 using RunType = R(Args...);

	 //Callback(std::function<RunType> func)
		//: func_(func){}

	 ////Callback(std::bind* )

	 //Callback(std::function<RunType>&& func) {
		// func_ = std::move(func);
	 //}

	 //operator std::function<RunType>() {
		// return func_;
	 //}



	 /*
	 OnceCallback(const OnceCallback&) = delete;
	 OnceCallback& operator=(OnceCallback&) = delete;

	 OnceCallback(OnceCallback&&) noexcept = default;
	 OnceCallback& operator=(OnceCallback&&) noexcept = default;
	 */

	 //bool Equals(const OnceCallback& other) const { return fun_ == other.fun_; }

	 R Run() const {
		 return 
	 }


 private:
	 //std::function<RunType> func_;
};

//using Closure = Callback<void()>;
using Closure = std::function<void()>;

}

#endif // !BASE_CALLBACK_H
