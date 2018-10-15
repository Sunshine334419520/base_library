/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: base_export.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_BASE_EXPORT_H
#define BASE_BASE_EXPORT_H


#if defined(COMPONENT_BUILD)
#if defined(OS_WIN)

#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __attribute__((visibility("default")))
#else
#define BASE_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define BASE_EXPORT
#endif



#endif // !BASE_BASE_EXPORT_H
