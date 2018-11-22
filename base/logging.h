/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: logging.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_LOGGING_H
#define BASE_LOGGING_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdint>

#include "base/base_export.h"

namespace logging {

enum class LogType {
	INFO,
	WARNING,
	ERROR,
	FATAL
};


class BASE_EXPORT Logging {
 public:
	 explicit Logging(LogType type) : type_(type) {}

	 ~Logging();

	 static std::ostream& Start(LogType type,
								const int32_t line,
								const std::string& fun_name);

	 static void InitLogger(const std::string& info_log_file_name,
							const std::string& warn_log_file_name,
							const std::string& error_log_file_name);
 private:
	 static std::ostream& GetStream(LogType type);

	 static std::ofstream info_log_file_;
	 static std::ofstream warn_log_file_;
	 static std::ofstream error_log_file_;
	 LogType type_;
};

}		// namespace logging.

#define LOG(log_type)	\
logging::Logging(log_type).Start(log_type, __LINE__, __FUNCTION__)

#define CHECK(val)		\
	if (!(val)) {		\
	LOG(logging::LogType::ERROR) << "CHECK failed" << std::endl		\
	<< #val << "=" << (val) << std::endl;							\
	abort();														\
	}

#define CHECK_NULL(val)				\
	if ((val) != NULL) {			\
	LOG(logging::LogType::ERROR) << "CHECK_NULL failed" << std::endl		\
	<< #val << "!=NULL" << std::endl;							\
	abort();		\
	}

#define CHECK_NOTNULL(val)		\
	if ((val) == NULL) {			\
	LOG(logging::LogType::ERROR) << "CHECK_NOTNULL failed" << std::endl		\
	<< #val << "==NULL" << std::endl;							\
	abort();		\
	}

#define CHECK_NE(val1, val2)		\
	if (!((val1) != (val2))) {			\
		LOG(logging::LogType::ERROR) << "CHECK_NE failed" \
		<< #val1 << "=" << (val1) << std::endl				\
		<< #val2 << "=" << (val2) << std::endl;			\
	}

#define CHECK_EQ(val1, val2)		\
	if (!((val1) == (val2))) {			\
		LOG(logging::LogType::ERROR) << "CHECK_EQ failed" \
		<< #val1 << "=" << (val1) << std::endl				\
		<< #val2 << "=" << (val2) << std::endl;			\
	}

#define CHECK_GE(val1, val2)		\
	if (!((val1) >= (val2))) {			\
		LOG(logging::LogType::ERROR) << "CHECK_GE failed" \
		<< #val1 << "=" << (val1) << std::endl				\
		<< #val2 << "=" << (val2) << std::endl;			\
	}

#define CHECK_LT(val1, val2)		\
	if (!((val1) < (val2))) {			\
		LOG(logging::LogType::ERROR) << "CHECK_LT failed" \
		<< #val1 << "=" << (val1) << std::endl				\
		<< #val2 << "=" << (val2) << std::endl;			\
	}


#if defined(NDEBUG)
#define DCHECK_IS_ON() 0
#else defined(DEBUG)
#define DCHECK_IS_ON() 1
#endif	// NDEBUG

#if DCHECK_IS_ON()

#define DCHECK(condition)		\
	CHECK(condition)

#define DCHECK_NOTNULL(val1)		\
	CHECK_NOTNULL(val1)			

#define DCHECK_NULL(val1)			\
	CHECK_NULL(val1)			

#define	DCHECK_EQ(val1, val2)				\
	CHECK_EQ(val1, val2)					\

#define DCHECK_NE(val1 ,val2)				\
	CHECK_NE(val1, val2)

#define DCHECK_GE(val1 ,val2)				\
	CHECK_GE(val1, val2)

#define DCHECK_LT(val1 ,val2)				\
	CHECK_LT(val1, val2)

#else 

#define DCHECK(condition)	
#define DCHECK_NOTNULL(val1)
#define DCHECK_NULL(val1)
#define DCHECK_EQ(val1, val2)
#define DCHECK_NE(val1, val2)
#define DCHECK_GE(val1, val2)
#define DCHECK_LT(val1, val2)


#endif		// DCHECK_IS_ON();

#endif // !BASE_LOGGING_H
