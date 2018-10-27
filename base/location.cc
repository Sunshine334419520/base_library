/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: location.cc
* @Last modified by:  YangGuang
*/

#include "base/location.h"

#include <stdio.h>

#include "base/macor.h"
#include "base/logging.h"

namespace base {

Location::Location() = default;
Location::Location(const Location& other) = default;

Location::Location(const char* file_name, const void* program_counter)
	: file_name_(file_name), program_counter_(program_counter) {}

Location::Location(const char* function_name,
				   const char* file_name,
				   int line_number,
				   const void* program_counter)
	: function_name_(function_name),
	file_name_(file_name),
	line_number_(line_number),
	program_counter_(program_counter) {
#if !defined(OS_NACL)
	// The program counter should not be null except in a default constructed
	// (empty) Location object. This value is used for identity, so if it doesn't
	// uniquely identify a location, things will break.
	//
	// The program counter isn't supported in NaCl so location objects won't work
	// properly in that context.
	//DCHECK(program_counter);
	DCHECK(program_counter);
#endif
}

std::string Location::ToString() const {
	if (has_source_info()) {
		return std::string(function_name_) + "@" + file_name_ + ":" +
			std::to_string(line_number_);
	}
	//return printf("pc:%p", program_counter_);
	return std::string(reinterpret_cast<const char*>(program_counter_));
}

#if defined(COMPILER_MSVC)
#define RETURN_ADDRESS() _ReturnAddress()
#elif defined(COMPILER_GCC) && !defined(OS_NACL)
#define RETURN_ADDRESS() \
  __builtin_extract_return_addr(__builtin_return_address(0))
#else
#define RETURN_ADDRESS() nullptr
#endif

// static
Location Location::CreateFromHere(const char* file_name) {
	return Location(file_name, RETURN_ADDRESS());
}

// static
Location Location::CreateFromHere(const char* function_name,
										   const char* file_name,
										   int line_number) {
	return Location(function_name, file_name, line_number, RETURN_ADDRESS());
}

//------------------------------------------------------------------------------
const void* GetProgramCounter() {
	return RETURN_ADDRESS();
}

}		// namespace base.