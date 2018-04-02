/*------------------------------------------------------*/
/* Thread base class                                    */
/*                                                      */
/* QtThread.h                                           */
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

#ifndef QTTHREAD_H
#define QTTHREAD_H

#include "QtThreadManager.h"
#include "QtAtomicOperationT.h"

class IQtEventQueue;
class IQtTimerQueue;
class IQtReactor;
class CQtEventThread;

class QT_OS_EXPORT AQtThread
{
public:
	QT_THREAD_ID GetThreadId();
	CQtThreadManager::TType GetThreadType();
	QT_THREAD_HANDLE GetThreadHandle();
	void Terminate();

	// Create thread.
	// The function won't return until OnThreadInit() returns.
	virtual QtResult Create(
		CQtThreadManager::TType aType, 
		CQtThreadManager::TFlag aFlag = CQtThreadManager::TF_JOINABLE);

	// Stop thread so that let the thread function return.
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);

	// Wait until the thread function return.
	QtResult Join();

	// Delete this.
	QtResult Destory(QtResult aReason);

	virtual void OnThreadInit();
	virtual void OnThreadRun() = 0;

	virtual IQtReactor* GetReactor();
	virtual IQtEventQueue* GetEventQueue();
	virtual IQtTimerQueue* GetTimerQueue();

	virtual void SetStop()
	{
		m_bStopFlag = TRUE;
	}

	virtual BOOL GetStopFlag()
	{
		return m_bStopFlag;
	}

protected:
	AQtThread();
	virtual ~AQtThread();

private:
#ifdef QT_WIN32
	static unsigned WINAPI ThreadProc(void *aPara);
#else
	static void* ThreadProc(void *aPara);
#endif // QT_WIN32

protected:
	QT_THREAD_ID m_Tid;
	QT_THREAD_HANDLE m_Handle;
	CQtThreadManager::TType m_Type;
	CQtThreadManager::TFlag m_Flag;

private:
	CQtEventThread *m_pEvent4Start;
	BOOL m_bRegistered;
	CQtAtomicOperationT<CQtMutexThread> m_NeedDelete;

	BOOL m_bStopFlag;
};

#endif // !QTTHREAD_H
