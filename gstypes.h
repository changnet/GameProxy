/*
 * 服务器基本数据类型。
 * 先将常用数据类型char short int long float 之类的转换为固定长度类型
 *
 * 在32/64bit系统下，8/16/32位数据基本不会有问题，但在32bit下，64位数据定义为long long int
 * 在64bit下，64位数据定义为long int，但long long int仍为64位.因此，使用int64_t能保证是64bit
 * 但对于sprintf之类的则仍会有警告。因此统一采用long long int。
 * PC现在不用16bit，不用考虑int为16bit的情况
 *
 * 参考Qt:
 * Typedef for signed char. This type is guaranteed to be 8-bit on all platforms supported by Qt.
 * Typedef for signed short. This type is guaranteed to be 16-bit on all platforms supported by Qt.
 * Typedef for signed int. This type is guaranteed to be 32-bit on all platforms supported by Qt.
 * Typedef for long long int (__int64 on Windows). This type is guaranteed to be 64-bit on all platforms supported by Qt.
 *
 *
 *about NULL,from Bjarne Stroustrup:
 *    In C++, the definition of NULL is 0, so there is only an aesthetic difference. I prefer to avoid macros,
 *so I use 0. Another problem with NULL is that people sometimes mistakenly believe that it is different
 *from 0 and/or not an integer. In pre-standard code, NULL was/is sometimes defined to something unsuitable
 *and therefore had/has to be avoided. That's less common these days.
 *    If you have to name the null pointer, call it nullptr; that's what it's going to be called in C++0x.
 *Then, "nullptr" will be a keyword.
 *
 */

#ifndef __GSTYPES_H__
#define __GSTYPES_H__

#include <string>    /*  std::string */
#include <cstring>   /*  for strerr */
#include <cerrno>    /*  for errno  */
#include <cassert>   /*  for assert */

# define UNUSED(x) (void)x

#ifdef NULL
#undef NULL		/* in case <stdio.h> has defined it. or stddef.h */
#endif

#if __cplusplus < 201103L    //-std=gnu99
    #include <stdint.h>
    #define NULL    0
#else
    #include <cstdint>    //if support C++ 2011
    #define NULL    nullptr
#endif

//char short int long already typedef,see /usr/include/stdint.h for more

/*
// Exact integral types.

// Signed.  //

// There is some amount of overlap with <sys/types.h> as known by inet code //
#ifndef __int8_t_defined
# define __int8_t_defined
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;
# if __WORDSIZE == 64
typedef long int		int64_t;
# else
__extension__
typedef long long int		int64_t;
# endif
#endif

// Unsigned.  //
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
# define __uint32_t_defined
#endif
#if __WORDSIZE == 64
typedef unsigned long int	uint64_t;
#else
__extension__
typedef unsigned long long int	uint64_t;
#endif



*/

//base type
typedef uint8_t        uint8;
typedef int8_t          int8;
typedef uint16_t      uint16;
typedef int16_t        int16;
typedef uint32_t      uint32;
typedef int32_t        int32;
typedef unsigned long long int	uint64;
typedef long long int	int64;

/*
 * use std::string instead of C strings,you may use char array like char buff[].
 * don.t use c type strings in cstring(string.h)
 * for my test,a std::string like "code\0more",c_str() return code\0,size() return 4.
 * test like this:
 * string strTest = "code\0more";
 * if ( '\0' == strTest.c_str()[ strTest.size() ] )
 *    cout << "yes" <<endl;
 */
typedef std::string    string;

#define null    NULL

//typedef true    true
//typedef false   false
//typedef bool       bool
//typedef float      float
//typedef double     double

typedef uint16    nethead;/* 定义网络消息头类型 */
typedef uint16    msghead;/* 定义共享内存通信消息头类型 */
typedef uint16    strhead;/* 定义字符串类型传输过程中消息头类型 */

#endif    //for __GSTYPES_H__
