/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: logging.cc
* @Last modified by:  YangGuang
*/
#include "base/logging.h"

#include <time.h>
#include <cstdlib>
#include <chrono>

namespace logging{


std::ofstream Logging::error_log_file_;
std::ofstream Logging::info_log_file_;
std::ofstream Logging::warn_log_file_;


Logging::~Logging() {
	GetStream(type_) << std::endl << std::flush;

	if (LogType::FATAL == type_) {
		/*
		info_log_file_.close();
		error_log_file_.close();
		_log_file_.close();
		*/
	}
}

// static method.
std::ostream & Logging::Start(LogType type,
							  const int32_t line,
							  const std::string & fun_name) {
	/*
	time_t tm;
	time(&tm);
	char time_string[128];
	ctime_r(&tm, time_string);
	*/
	auto now_time = std::chrono::system_clock::now();
	return GetStream(type) << now_time.time_since_epoch().count()
		<< "\tfunction (" << fun_name << ")"
		<< "\tline" << line << "\t"
		<< std::flush;
	
}

// static method.
void Logging::InitLogger(const std::string&info_log_filename,
						 const std::string&warn_log_filename,
						 const std::string&error_log_filename) {
	info_log_file_.open(info_log_filename.c_str());
	warn_log_file_.open(warn_log_filename.c_str());
	error_log_file_.open(error_log_filename.c_str());
}

// static method.
std::ostream & Logging::GetStream(LogType type) {
	if (type == LogType::INFO)
		return info_log_file_.is_open() ?
		info_log_file_ : std::cout;

	if (type == LogType::WARNING)
		return warn_log_file_.is_open() ?
		warn_log_file_ : std::cerr;

	if (type == LogType::ERROR)
		return error_log_file_.is_open() ?
		error_log_file_ : std::cerr;
	// TODO: 在此处插入 return 语句
	return std::cerr;
}


}