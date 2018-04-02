//#include <stdio.h>
#include "QtBase.h"
#include "QtMutex.h"
//////////////////////////////////////////////////////////////////////
// class CQtMutexThreadBase
//////////////////////////////////////////////////////////////////////

CQtMutexThreadBase::CQtMutexThreadBase()
{
}

CQtMutexThreadBase::~CQtMutexThreadBase()
{
#ifdef QT_WIN32
	::DeleteCriticalSection(&m_Lock);
#else
	int nRet = ::pthread_mutex_destroy(&m_Lock);
	if (nRet != 0)  {
		QT_ERROR_TRACE_THIS("CQtMutexThreadBase::~CQtMutexThreadBase, pthread_mutex_destroy() failed! err=" << nRet);
	}
#endif // QT_WIN32
}

QtResult CQtMutexThreadBase::Lock()
{

	int nRet = ::pthread_mutex_lock(&m_Lock);
	if (nRet == 0)
	{
		return QT_OK;
	}
	else {
		//QT_ERROR_TRACE_THIS("CQtMutexThreadBase::Lock, pthread_mutex_lock() failed! err=" << nRet);
		return QT_ERROR_FAILURE;
	}
}

QtResult CQtMutexThreadBase::UnLock()
{
#ifdef QT_WIN32
	::LeaveCriticalSection(&m_Lock);
	return QT_OK;
#else
	int nRet = ::pthread_mutex_unlock(&m_Lock);
	if (nRet == 0)
		return QT_OK;
	else {
		QT_ERROR_TRACE_THIS("CQtMutexThreadBase::UnLock, pthread_mutex_unlock() failed! err=" << nRet);
		return QT_ERROR_FAILURE;
	}
#endif // QT_WIN32
}

QtResult CQtMutexThreadBase::TryLock()
{
#ifdef QT_WIN32
	BOOL bRet = ::TryEnterCriticalSection(&m_Lock);
	return bRet ? QT_OK : QT_ERROR_FAILURE;
#else
	int nRet = ::pthread_mutex_trylock(&m_Lock);
	return (nRet == 0) ? QT_OK : QT_ERROR_FAILURE;
#endif // QT_WIN32
}


//////////////////////////////////////////////////////////////////////
// class CQtMutexThreadRecursive
//////////////////////////////////////////////////////////////////////

CQtMutexThreadRecursive::CQtMutexThreadRecursive()
{
#ifdef QT_WIN32
	::InitializeCriticalSection(&m_Lock);
#else
	pthread_mutexattr_t mutexattr;
    ::pthread_mutexattr_init(&mutexattr);
#if !defined QT_SOLARIS && !defined MachOSupport 
    ::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
    ::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
#endif
    int nRet = ::pthread_mutex_init(&m_Lock, &mutexattr);
    ::pthread_mutexattr_destroy(&mutexattr);
	if (nRet != 0)  {
		QT_ERROR_TRACE_THIS("CQtMutexThreadRecursive::CQtMutexThreadRecursive, pthread_mutex_init() failed! err=" << nRet);
	}
#endif // QT_WIN32
}

CQtMutexThreadRecursive::~CQtMutexThreadRecursive()
{
}


//////////////////////////////////////////////////////////////////////
// class CQtMutexThreadRecursive
//////////////////////////////////////////////////////////////////////

CQtMutexThread::CQtMutexThread()
{
#ifdef QT_WIN32
	::InitializeCriticalSection(&m_Lock);
#else
	pthread_mutexattr_t mutexattr;
    ::pthread_mutexattr_init(&mutexattr);
#ifndef QT_SOLARIS //todo: how is solaris
    ::pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_FAST_NP);
#endif
    int nRet = ::pthread_mutex_init(&m_Lock, &mutexattr);
    ::pthread_mutexattr_destroy(&mutexattr);
	if (nRet != 0)  {
		QT_ERROR_TRACE_THIS("CQtMutexThread::CQtMutexThread, pthread_mutex_init() failed! err=" << nRet);
	}
#endif // QT_WIN32
}

CQtMutexThread::~CQtMutexThread()
{
}
