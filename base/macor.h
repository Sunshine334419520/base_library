//
// Created by Sunshine on 2018/7/17.
//

#ifndef BASE_MACOR_H
#define BASE_MACOR_H


// 鎵€鍦ㄧ幆澧?
#if !defined(TINYCHAT_SERVER)

#define TINYCHAT_SERVER 1
#endif

// 鍚勪釜骞冲彴瀹忓畾涔?

#if defined(__native_client__)
#define OS_NACL 1
#elif defined(__APPLE__)
#define OS_MACOSX 1
#elif defined(__linux__)
#define OS_LINUX 1
#include <unistd.h>
#elif defined(_WIN32)
#define OS_WIN 1
#elif defined(__FreeBSD__)
#define OS_FREEBSD 1
#elif defined(__OpenBSD__)
#define OS_OPENBSD 1

#endif


#if defined(OS_MACOSX) || defined(OS_LINUX) ||  \
    defined(OS_FREEBSD) || defined(OS_OPENBSD)
#define OS_POSIX 1
#endif

#if defined(__GUNC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for you compiler in macor.h
#endif



// 鍚勪釜 C++ 鍩虹瀹忓畾涔?
#if !defined(OVERRIDE)

#define OVERRIDE override

#endif


#if !defined(DISALLOW_COPY_AND_ASSIGN)

#define DISALLOW_COPY_AND_ASSIGN(Class_Name) \
 private:      \
 Class_Name(const Class_Name&);         \
 void operator=(const Class_Name&)

#endif

#if !defined(DISALLOW_IMPLEMENT_AND_EXTEND)

#define DISALLOW_IMPLEMENT_AND_EXTEND(Class_Name) \
  Class_Name();         \
  ~Class_Name();            \
  DISALLOW_COPY_AND_ASSIGN(Class_Name)

#endif

#if !defined(DISALLOW_IMPLEMENT)

#define DISALLOW_IMPLEMENT(Class_Name) \
 protected:              \
  Class_Name();          \
  ~Class_Name();           \
  DISALLOW_COPY_AND_ASSIGN(Class_Name)

#endif

#if !defined(ALLOW_ONLY_HEAP_ALLOC)

#define ONLYALLOW_HEAP_ALLOC_AND_NOTEXTEND(Class_Name)   \
 private:         \
  ~Class_Name() {}

#endif

#if !defined(ONLYALLOW_HEAP_ALLOC)

#define ONLYALLOW_HEAP_ALLOC(Class_Name)        \
 protected:             \
  virtual ~Class_Name() {}

#endif





#endif //TINYCHATSERVER_MACOR_H
