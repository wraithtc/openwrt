#ifndef __QTD_MEMORY_MGR__
#define __QTD_MEMORY_MGR__

#include <stdlib.h>
#ifdef WIN32
#	include <malloc.h>
#endif

#ifndef __QT_MEM_CHECK__

#define QT_MALLOC	malloc
#define QT_FREE(p)			\
	do					\
	{					\
		if((p))			\
		{				\
			free((p));	\
			(p) = NULL;	\
		}				\
	}					\
	while(0)			\

#else//__QT_MEM_CHECK__

#include "QtDefines.h"

#pragma  warning (disable :4291)

extern "C" 
{
	void* QT_malloc(size_t len, BYTE* filename, long lineno); //it will set the memory to 0;
	void* QT_calloc(size_t nmemb, size_t size, BYTE* filename, long lineno);
	void* QT_realloc(void* ptr, size_t len, BYTE* filename, long lineno);
	void QT_free(void* ptr);
	void QT_memdump();
}

#define QT_MALLOC(l)		QT_malloc(l, __FILE__, __LINE__)
#define QT_FREE(p)			QT_free(p)
#define calloc(n, s)		QT_calloc(n, s, __FILE__, __LINE__)
#define realloc(p, l)		QT_realloc(p, l,__FILE__, __LINE__)

inline void* operator new(size_t size, BYTE* file, long lineno) throw ()
{
	return QT_malloc(size, file, lineno);
}

inline void* operator new[] (size_t size, BYTE* file, long lineno)
{
	return QT_malloc(size, file, lineno);
}

inline void operator delete(void *p)
{
	QT_free(p);
}

inline void operator delete[](void* p)
{
	QT_free(p);
}

#define new _MY_CRT_DEBUG_NEW
#define _MY_CRT_DEBUG_NEW new( __FILE__, __LINE__)

#endif //!__QT_MEM_CHECK__

#endif

