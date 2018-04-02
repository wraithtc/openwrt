/*------------------------------------------------------*/
/* Utility classes                                      */
/*                                                      */
/* QtUtilClasses.h                                      */
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

#ifndef QTUTILCLASSES_H
#define QTUTILCLASSES_H

#include "QtDefines.h"
#include "QtThreadInterface.h"
#include "QtStdCpp.h"

class CQtMutexThreadRecursive;
class AQtThread;

class QT_OS_EXPORT CQtEnsureSingleThread
{
public:
	CQtEnsureSingleThread();
	void EnsureSingleThread() const;
	void Reset2CurrentThreadId();
	void Reset2ThreadId(QT_THREAD_ID aTid);

protected:
	QT_THREAD_ID m_ThreadIdOpen;
};

class QT_OS_EXPORT CQtStopFlag
{
public:
	CQtStopFlag() : m_bStoppedFlag(TRUE)
	{
	}

	void SetStartFlag();

	void SetStopFlag();

	BOOL IsFlagStopped() const
	{
		m_Est.EnsureSingleThread();
		return m_bStoppedFlag;
	}
	
	CQtEnsureSingleThread m_Est;
	BOOL m_bStoppedFlag;
};

class QT_OS_EXPORT CQtSignalStop : public IQtEvent
{
public:
	static CQtSignalStop* Instance();

	virtual ~CQtSignalStop();

	// Do the stop work.
	QtResult Launch(int aSig);
	
	// interface IQtEvent.
	virtual QtResult OnEventFire();

	virtual void OnDestorySelf();

private:
	CQtSignalStop();

	static CQtSignalStop s_SignalStopSingleton;
	AQtThread *m_pThread;
};

class CQtErrnoGuard
{
public:
	CQtErrnoGuard() : m_nErr(errno)
	{
	}

	~CQtErrnoGuard()
	{
#ifdef MACOS
	#ifndef MachOSupport
		CFM_seterrno(m_nErr);
	#endif	//MachOSupport
#else
		errno = m_nErr;
#endif
	}
private:
	int m_nErr;
};

class QT_OS_EXPORT CQtCleanUpBase
{
public:
	static void CleanupAll();

protected:
	CQtCleanUpBase();
	virtual ~CQtCleanUpBase();
	virtual void CleanUp();

private:
	CQtCleanUpBase *m_pNext;
	static CQtCleanUpBase *s_pHeader;

private:
	typedef CQtMutexThreadRecursive MutexType;
	
	// = Prevent assignment and initialization.
	void operator = (const CQtCleanUpBase&);
	CQtCleanUpBase(const CQtCleanUpBase&);
};

class QT_OS_EXPORT CQtDataBlockNoMalloc
{
public:
	CQtDataBlockNoMalloc(LPCSTR aStr, DWORD aLen);

	/// Read and advance <aCount> bytes, 
	QtResult Read(LPVOID aDst, DWORD aCount, DWORD *aBytesRead = NULL);

	/// Write and advance <aCount> bytes
	QtResult Write(LPCVOID aSrc, DWORD aCount, DWORD *aBytesWritten = NULL);
	
private:
	LPCSTR m_pBegin;
	LPCSTR m_pEnd;
	LPCSTR m_pCurrentRead;
	LPSTR m_pCurrentWrite;
};

#endif // !QTUTILCLASSES_H
