/*------------------------------------------------------*/
/* the basic definitions                                */
/*                                                      */
/* QtBase.h                                             */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

#ifndef QTDEFINES_H
#define QTDEFINES_H
#define LINUX

//////////////////////////////////////////////////////////////////////
// First definition: choose OS
//////////////////////////////////////////////////////////////////////

#ifdef UNIX
  #ifndef QT_UNIX
    #define QT_UNIX
  #endif // QT_UNIX
#endif // UNIX

#ifdef LINUX
  #ifndef QT_LINUX
    #define QT_LINUX
  #endif // QT_LINUX
  #ifndef QT_UNIX
    #define QT_UNIX
  #endif // QT_UNIX
#endif // LINUX

#ifdef MACOS
  #ifndef QT_MACOS
    #define QT_MACOS
  #endif // QT_MACOS
#endif // MACOS

typedef long QtResult;

//////////////////////////////////////////////////////////////////////
// OS API definition
//////////////////////////////////////////////////////////////////////

#ifdef QT_MACOS
	typedef int sem_t;
	enum
	{
	  PTHREAD_MUTEX_TIMED_NP,
	  PTHREAD_MUTEX_RECURSIVE_NP,
	  PTHREAD_MUTEX_ERRORCHECK_NP,
	  PTHREAD_MUTEX_ADAPTIVE_NP
	  
	  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_ADAPTIVE_NP
	};
	#endif
	
#ifdef QT_WIN32

#ifdef QT_ENABLE_SELECT_WINOS
#define QT_ENABLE_DNS_THREAD
#endif //QT_ENABLE_SELECT_WINOS
//  #ifndef NOMINMAX
//    #define NOMINMAX
//  #endif // NOMINMAX

  // supports Windows NT 4.0 and later, not support Windows 95.
  // mainly for using winsock2 functions
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0400
  #endif // _WIN32_WINNT
#if _MSC_VER <= 1200
//#define WIN32_LEAN_AND_MEAN 
  #include <windows.h>
  #include <winsock2.h>
#else
  #include <winsock2.h>
  #include <windows.h>
#endif

  // The ordering of the fields in this struct is important. 
  // It has to match those in WSABUF.
  struct iovec
  {
    u_long iov_len; // byte count to read/write
    char *iov_base; // data to be read/written
  };

  #define EWOULDBLOCK             WSAEWOULDBLOCK
  #define EINPROGRESS             WSAEINPROGRESS
  #define EALREADY                WSAEALREADY
  #define ENOTSOCK                WSAENOTSOCK
  #define EDESTADDRREQ            WSAEDESTADDRREQ
  #define EMSGSIZE                WSAEMSGSIZE
  #define EPROTOTYPE              WSAEPROTOTYPE
  #define ENOPROTOOPT             WSAENOPROTOOPT
  #define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
  #define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
  #define EOPNOTSUPP              WSAEOPNOTSUPP
  #define EPFNOSUPPORT            WSAEPFNOSUPPORT
  #define EAFNOSUPPORT            WSAEAFNOSUPPORT
  #define EADDRINUSE              WSAEADDRINUSE
  #define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
  #define ENETDOWN                WSAENETDOWN
  #define ENETUNREACH             WSAENETUNREACH
  #define ENETRESET               WSAENETRESET
  #define ECONNABORTED            WSAECONNABORTED
  #define ECONNRESET              WSAECONNRESET
  #define ENOBUFS                 WSAENOBUFS
  #define EISCONN                 WSAEISCONN
  #define ENOTCONN                WSAENOTCONN
  #define ESHUTDOWN               WSAESHUTDOWN
  #define ETOOMANYREFS            WSAETOOMANYREFS
  #define ETIMEDOUT               WSAETIMEDOUT
  #define ECONNREFUSED            WSAECONNREFUSED
  #define ELOOP                   WSAELOOP
  #define EHOSTDOWN               WSAEHOSTDOWN
  #define EHOSTUNREACH            WSAEHOSTUNREACH
  #define EPROCLIM                WSAEPROCLIM
  #define EUSERS                  WSAEUSERS
  #define EDQUOT                  WSAEDQUOT
  #define ESTALE                  WSAESTALE
  #define EREMOTE                 WSAEREMOTE
#endif // QT_WIN32

#ifdef QT_WIN32
  typedef HANDLE QT_HANDLE;
  typedef SOCKET QT_SOCKET;
  #define QT_INVALID_HANDLE INVALID_HANDLE_VALUE
  #define QT_SD_RECEIVE SD_RECEIVE
  #define QT_SD_SEND SD_SEND
  #define QT_SD_BOTH SD_BOTH
#else // !QT_WIN32
  typedef int QT_HANDLE;
  typedef QT_HANDLE QT_SOCKET;
  #define QT_INVALID_HANDLE -1
  #define QT_SD_RECEIVE 0
  #define QT_SD_SEND 1
  #define QT_SD_BOTH 2
#endif // QT_WIN32

#ifdef QT_MACOS
  #include "atdefs.h"
  typedef float                 FLOAT;
  
  typedef const void           *LPCVOID;
 #ifndef MachOSupport 
  struct iovec {
        char   *iov_base;  /* Base address. */
        unsigned long iov_len;    /* Length. */
  };
 #endif	//MachOSupport
#endif // QT_MACOS

#ifdef QT_UNIX 
  typedef long long           LONGLONG;
  typedef unsigned long       DWORD;
  typedef long                LONG;
  typedef int                 BOOL;
  typedef unsigned char       BYTE;
  typedef unsigned short        WORD;
  typedef float                 FLOAT;
  typedef int                   INT;
  typedef unsigned int          UINT;
  typedef FLOAT                *PFLOAT;
  typedef BOOL                 *LPBOOL;
  typedef int                  *LPINT;
  typedef WORD                 *LPWORD;
  typedef long                 *LPLONG;
  typedef DWORD                *LPDWORD;
  typedef unsigned int         *LPUINT;
  typedef void                 *LPVOID;
  typedef const void           *LPCVOID;
  typedef char                  CHAR;
  typedef char                  TCHAR;
  typedef unsigned short        WCHAR;
  typedef const char           *LPCSTR;
  typedef char                 *LPSTR;
  typedef const unsigned short *LPCWSTR;
  typedef unsigned short       *LPWSTR;
  typedef BYTE                 *LPBYTE;
  typedef const BYTE           *LPCBYTE;
  
  #ifndef FALSE
    #define FALSE 0
  #endif // FALSE
  #ifndef TRUE
    #define TRUE 1
  #endif // TRUE
#endif // !QT_UNIX

#ifdef QT_SOLARIS
  #define INADDR_NONE             0xffffffff
#endif

#ifdef _MSC_VER
  #ifndef _MT
    #error Error: please use multithread version of C runtime library.
  #endif // _MT

  #pragma warning(disable: 4786) // identifier was truncated to '255' characters in the browser information(mainly brought by stl)
  #pragma warning(disable: 4355) // disable 'this' used in base member initializer list
  #pragma warning(disable: 4275) // deriving exported class from non-exported
  #pragma warning(disable: 4251) // using non-exported as public in exported
#endif // _MSC_VER

#ifdef QT_WIN32
  #if defined (_LIB) || (QT_OS_BUILD_LIB) 
	//#define QT_OS_EXPORT __declspec(dllimport)
	#define QT_OS_EXPORT
  #else 
    #if defined (_USRDLL) || (QT_OS_BUILD_DLL)
      #define QT_OS_EXPORT __declspec(dllexport)
    #else 
      #define QT_OS_EXPORT __declspec(dllimport)
    #endif // _USRDLL || QT_OS_BUILD_DLL
  #endif // _LIB || QT_OS_BUILD_LIB
#else
  #define QT_OS_EXPORT 
#endif // !QT_WIN32

#if defined (QT_WIN32)
  #define QT_OS_SEPARATE '\\'
#elif defined (QT_UNIX) || defined(QT_MACOS)
  #define QT_OS_SEPARATE '/'
#endif

#define QT_BIT_ENABLED(dword, bit) (((dword) & (bit)) != 0)
#define QT_BIT_DISABLED(dword, bit) (((dword) & (bit)) == 0)
#define QT_BIT_QTP_MASK(dword, bit, mask) (((dword) & (bit)) == mask)
#define QT_SET_BITS(dword, bits) (dword |= (bits))
#define QT_CLR_BITS(dword, bits) (dword &= ~(bits))


//////////////////////////////////////////////////////////////////////
// C definition
//////////////////////////////////////////////////////////////////////

#ifdef QT_WIN32
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>
  #include <limits.h>
  #include <stddef.h>
  #include <stdarg.h>
  #include <signal.h>
  #include <errno.h>
  #include <wchar.h>

  #include <crtdbg.h>
  #include <process.h>
  #define getpid _getpid
  #define snprintf _snprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
  #define vsnprintf _vsnprintf
#endif // QT_WIN32

#ifdef QT_UNIX
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>
  #include <limits.h>
  #include <stdarg.h>
  #include <time.h>
  #include <signal.h>
  #include <sys/stat.h>
  #include <sys/fcntl.h>
  #include <pthread.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <ctype.h>
  
  #define EWOULDBLOCK EAGAIN
 
  #include <assert.h>

  #include <netinet/tcp.h>
  #include <semaphore.h>
#endif // QT_UNIX

#ifdef QT_SOLARIS
  #include <sys/filio.h>
#endif//QT_SOLARIS


#ifdef QT_WIN32
#define QT_IOV_MAX 64
#else
// This is defined by XOPEN to be a minimum of 16.  POSIX.1g
// also defines this value.  platform-specific config.h can
// override this if need be.
#if !defined (IOV_MAX)
#define IOV_MAX 16
#endif // !IOV_MAX
#define QT_IOV_MAX IOV_MAX
#endif // QT_WIN32

#ifdef QT_WIN32
	typedef DWORD QT_THREAD_ID;
	typedef HANDLE QT_THREAD_HANDLE;
	typedef HANDLE QT_SEMAPHORE_T;
	typedef CRITICAL_SECTION QT_THREAD_MUTEX_T;
#else // !QT_WIN32
	typedef pthread_t QT_THREAD_ID;
	typedef QT_THREAD_ID QT_THREAD_HANDLE;
	typedef sem_t QT_SEMAPHORE_T;
	typedef pthread_mutex_t QT_THREAD_MUTEX_T;
#endif // QT_WIN32

#ifdef QT_MACOS
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #ifndef MachOSupport
  #include <DateTimeUtils.h>
  	#include <CGBase.h>
	#include <ctype.h>
#else
  	#include <pthread.h>
  	#include <unistd.h>
  	#include <sys/uio.h>
  	#include <netinet/in.h>
  	#include <arpa/inet.h>
  	#include <sys/types.h>
  	#include <sys/socket.h>
  	#include <netdb.h>
  	#include <semaphore.h>
  	#include <sys/resource.h>
  	#include <sys/ioctl.h>
    #include <sys/stat.h>
    #include <sys/fcntl.h>
  #endif	//MachOSupport
  #include <utime.h>
  //#include <cstdint>

  
  
  #ifndef MachOSupport	
  #include "CFMCallSysBundle.h"
  #include "sys-socket.h"
  #include "netinet-in.h"
  #include "netdb.h"
  #include "fcntl.h"
  #endif	//MachOSupport
#endif // QT_MACOS

 
//////////////////////////////////////////////////////////////////////
// Assert
//////////////////////////////////////////////////////////////////////

#ifdef QT_WIN32
  #include <crtdbg.h>
  #ifdef _DEBUG
    #define QT_DEBUG
  #endif // _DEBUG

  #if defined (QT_DEBUG)
    #define QT_ASSERTE _ASSERTE
  #endif // QT_DEBUG
#endif // QT_WIN32

#ifdef QT_UNIX
  #include <assert.h>
  #if defined (QT_DEBUG) && !defined (QT_DISABLE_ASSERTE)
    #define QT_ASSERTE assert
  #endif // QT_DEBUG
#endif // QT_UNIX

#ifdef QT_DISABLE_ASSERTE
#include "QtDebug.h"
  #ifdef QT_ASSERTE
	#undef QT_ASSERTE
  #endif

#ifndef QT_WIN32
  #define QT_ASSERTE(expr) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
		} \
	} while (0)

#else
  #define QT_ASSERTE(expr) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
		} \
	} while (0)
#endif // !QT_WIN32
#endif // QT_DISABLE_ASSERTE

#ifndef QT_ASSERTE
  #define QT_ASSERTE(expr) 
#endif // QT_ASSERTE

//#define QT_ASSERTE_THROW QT_ASSERTE

#ifdef QT_DISABLE_ASSERTE
#ifdef QT_WIN32
  #define QT_ASSERTE_RETURN(expr, rv) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return rv; \
		} \
	} while (0)

  #define QT_ASSERTE_RETURN_VOID(expr) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return; \
		} \
	} while (0)
#else
  #define QT_ASSERTE_RETURN(expr, rv) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return rv; \
		} \
	} while (0)

  #define QT_ASSERTE_RETURN_VOID(expr) \
	do { \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return; \
		} \
	} while (0)

#endif //QT_WIN32
#else
#ifdef QT_WIN32
  #define QT_ASSERTE_RETURN(expr, rv) \
	do { \
		QT_ASSERTE((expr)); \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return rv; \
		} \
	} while (0)

  #define QT_ASSERTE_RETURN_VOID(expr) \
	do { \
		QT_ASSERTE((expr)); \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return; \
		} \
	} while (0)
#else
  #define QT_ASSERTE_RETURN(expr, rv) \
	do { \
		QT_ASSERTE((expr)); \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return rv; \
		} \
	} while (0)

  #define QT_ASSERTE_RETURN_VOID(expr) \
	do { \
		QT_ASSERTE((expr)); \
		if (!(expr)) { \
			QT_ERROR_TRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
			return; \
		} \
	} while (0)


#endif//QT_WIN32
#endif // QT_DISABLE_ASSERTE

#endif // !QTDEFINES_H
