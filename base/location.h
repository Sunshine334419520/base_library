/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: location.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_LOCATION_H
#define BASE_LOCATION_H

#include <stddef.h>

#include <cassert>
#include <string>

#include "base/base_export.h"
#include "base/hash.h"

namespace base {

class BASE_EXPORT Location {
 public:
	 Location();
	 Location(const Location& other);

	 // 只初始化file name 和 program counter
	 Location(const char* file_name, const void* program_counter);

	 // 应该提供一个长时间存在的char*，如__FILE__.这个值将不会产生复制,
	 // 只是简单的指针之间的赋值.
	 Location(const char* function_name,
			  const char* file_name,
			  int line_number,
			  const void* program_counter);

	 // 用于hash map insertion的比较器, 这个program counter应该是唯一标识
	 // 这个Location.
	 bool operator==(const Location& other) const {
		return program_counter_ == other.program_counter_;
	 }

	 // 如果放回true代表有source code location info. 返回false，
	 // 这个Location对象只包含一个program counter,或者是default-initialized.
	 bool has_source_info() const { return function_name_ && file_name_; }

	 // 如果是default initialize，返回值将是nullptr.
	 const char* function_name() const { return function_name_; }

	 // 如果是default initialize, 返回值将是nullptr.
	 const char* file_name() const { return file_name_; }

	 // 如果是default initialize,返回值将是-1.
	 int line_number() const { return line_number_; }

	 // 这个program_counter应该是一直有效的，除非是defult initialize.
	 // 那样返回值将是nullptr.
	 const void* program_counter() const { return program_counter_; }

	 // 转换成用户最能够看懂的样子, 如果function and filename 是无效的，
	 // 这个将return "pc:<hex address>".
	 std::string ToString() const;

	 static Location CreateFromHere(const char* file_name);
	 static Location CreateFromHere(const char* function_name,
									const char* file_name,
									int line_number);

 private:
	 const char* function_name_ = nullptr;
	 const char* file_name_ = nullptr;
	 int line_number_ = -1;
	 const void* program_counter_ = nullptr;
};

BASE_EXPORT const void* GetProgramCounter();

//// The macros defined here will expand to the current function.
//#if BUILDFLAG(ENABLE_LOCATION_SOURCE)
//
//#define FROM_HERE FROM_HERE_WITH_EXPLICIT_FUNCTION(__func__)
//#define FROM_HERE_WITH_EXPLICIT_FUNCTION(function_name)	\
//	::base::Location::CreateFromHere(function_name, __FILE__, __LINE__)
//
//#else 

#define FROM_HERE ::base::Location::CreateFromHere(__FILE__)
#define FROM_HERE_WITH_EXPLICIT_FUNCTION(function_name) \
	::base::Location::CreateFromHere(function_name, __FILE__, -1)

}	// namespace base.

namespace std {

template <>
struct hash<base::Location> {
	std::size_t operator()(const base::Location& loc) const {
		const void* program_counter = loc.program_counter();
		return base::Hash(&program_counter, sizeof(void*));
	}
};

}


#endif // !BASE_LOCATION_H
