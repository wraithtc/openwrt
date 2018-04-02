/*------------------------------------------------------*/
/* Mutex classes                                        */
/*                                                      */
/* QtMutex.h                                            */
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

#ifndef QTMUTEX_H
#define QTMUTEX_H

#include "QtDefines.h"
#include "QtError.h"
#include "QtUtilClasses.h"

class QT_OS_EXPORT CQtMutexThreadBase
{
protected:
	CQtMutexThreadBase();
	virtual ~CQtMutexThreadBase();

public:
	QtResult Lock();
	QtResult UnLock();
	QtResult TryLock();

	QT_THREAD_MUTEX_T& GetMutexType() { return m_Lock;}

protected:
	QT_THREAD_MUTEX_T m_Lock;
};

/**
 * Mainly copyed from <ACE_Recursive_Thread_Mutex>.
 * <CQtMutexThreadRecursive> allows mutex locking many times in the same threads.
 */
class QT_OS_EXPORT CQtMutexThreadRecursive : public CQtMutexThreadBase
{
public:
	CQtMutexThreadRecursive();
	~CQtMutexThreadRecursive();

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtMutexThreadRecursive&);
	CQtMutexThreadRecursive(const CQtMutexThreadRecursive&);
};

class QT_OS_EXPORT CQtMutexThread : public CQtMutexThreadBase
{
public:
	CQtMutexThread();
	~CQtMutexThread();

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtMutexThread&);
	CQtMutexThread(const CQtMutexThread&);
};

/**
 *	<CQtMutexNullSingleThread> checks to ensure running on the same thread
 */
class QT_OS_EXPORT CQtMutexNullSingleThread
{
public:
	CQtMutexNullSingleThread() 
	{
	}

	// this function may be invoked in the different thread.
	~CQtMutexNullSingleThread() 
	{
//		m_Est.EnsureSingleThread();
	}

	QtResult Lock() 
	{
		m_Est.EnsureSingleThread();
		return QT_OK;
	}

	QtResult UnLock() 
	{
		m_Est.EnsureSingleThread();
		return QT_OK;
	}

	QtResult TryLock() 
	{
		m_Est.EnsureSingleThread();
		return QT_OK;
	}

private:
	CQtEnsureSingleThread m_Est;

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtMutexNullSingleThread&);
	CQtMutexNullSingleThread(const CQtMutexNullSingleThread&);
};

// Use <CQtMutexNullSingleThread> instead <CQtMutexNull> to ensure in the single thread.
#if 1 
/**
 *	<CQtMutexNull> runs on different threads without operating and checking
 */
class QT_OS_EXPORT CQtMutexNull
{
public:
	CQtMutexNull() 
	{
	}
	
	~CQtMutexNull() 
	{
	}

	QtResult Lock() 
	{
		return QT_OK;
	}

	QtResult UnLock() 
	{
		return QT_OK;
	}

	QtResult TryLock() 
	{
		return QT_OK;
	}

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtMutexNull&);
	CQtMutexNull(const CQtMutexNull&);
};
#endif

template <class MutexType>
class QT_OS_EXPORT CQtMutexGuardT
{
public:
	CQtMutexGuardT(MutexType& aMutex)
		: m_Mutex(aMutex)
		, m_bLocked(FALSE)
	{
		Lock();
	}

	~CQtMutexGuardT()
	{
		UnLock();
	}

	QtResult Lock() 
	{
		QtResult rv = m_Mutex.Lock();
		m_bLocked = QT_SUCCEEDED(rv) ? TRUE : FALSE;
		return rv;
	}

	QtResult UnLock() 
	{
		if (m_bLocked) {
			m_bLocked = FALSE;
			return m_Mutex.UnLock();
		}
		else {
			return QT_OK;
		}
	}

private:
	MutexType& m_Mutex;
	BOOL m_bLocked;

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtMutexGuardT&);
	CQtMutexGuardT(const CQtMutexGuardT&);
};

#endif // !QTMUTEX_H
