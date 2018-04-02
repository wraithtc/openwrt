/*------------------------------------------------------*/
/* EventQueue base class and synchronous event          */
/*                                                      */
/* QtEventQueueBase.h                                   */
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

#ifndef QTEVENTQUEUEBASE_H
#define QTEVENTQUEUEBASE_H

#include "QtReactorInterface.h"
#include "QtConditionVariable.h"
#include "QtTimeValue.h"
#include <list>

class CQtEventQueueBase;

class CQtEventSynchronous : public IQtEvent
{
public:
	CQtEventSynchronous(IQtEvent *aEventPost, CQtEventQueueBase *aEventQueue);
	virtual ~CQtEventSynchronous();

	// interface IQtEvent
	virtual QtResult OnEventFire();
	virtual void OnDestorySelf();

	QtResult WaitResultAndDeleteThis();

private:
	IQtEvent *m_pEventPost;
	QtResult m_Result;
	CQtEventQueueBase *m_pEventQueue;
	BOOL m_bHasDestoryed;
	CQtEventThread m_SendEvent;
};

class QT_OS_EXPORT CQtEventQueueBase : public IQtEventQueue
{
public:
	CQtEventQueueBase();
	virtual ~CQtEventQueueBase();

	// interface IQtEventQueue
	virtual QtResult PostEvent(IQtEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);
	virtual QtResult SendEvent(IQtEvent *aEvent);
	virtual DWORD GetPendingEventsCount();

	void Stop();
	
	void DestoryPendingEvents();

	void Reset2CurrentThreadId();

	enum { MAX_GET_ONCE = 5 };
	typedef std::list<IQtEvent *> EventsType;

	// Don't make the following two functions static because we want trace size.
	QtResult ProcessEvents(const EventsType &aEvents);
	QtResult ProcessOneEvent(IQtEvent *aEvent);

	static CQtTimeValue s_tvReportInterval;

protected:
	QtResult PopPendingEvents(
		EventsType &aEvents, 
		DWORD aMaxCount = MAX_GET_ONCE, 
		DWORD *aRemainSize = NULL);

	QtResult PopOnePendingEvent(
		IQtEvent *&aEvent, 
		DWORD *aRemainSize = NULL);
	
	EventsType m_Events;
	// we have to record the size of events list due to limition of std::list in Linux.
	DWORD m_dwSize;
	CQtTimeValue m_tvReportSize;

private:
	QT_THREAD_ID m_Tid;
	BOOL m_bIsStopped;

	friend class CQtEventSynchronous;
};

class CQtEventQueueUsingMutex : public CQtEventQueueBase 
{
public:
	CQtEventQueueUsingMutex();
	virtual ~CQtEventQueueUsingMutex();

	// interface IQtEventQueue
	virtual QtResult PostEvent(IQtEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);

	// Pop <aMaxCount> pending events in the queue, 
	// if no events are pending, return at once.
	QtResult PopPendingEventsWithoutWait(
		CQtEventQueueBase::EventsType &aEvents, 
		DWORD aMaxCount = MAX_GET_ONCE, 
		DWORD *aRemainSize = NULL);

	// Pop one pending events, and fill <aRemainSize> with remain size.
	// if no events are pending, return at once.
	QtResult PopOnePendingEventWithoutWait(
		IQtEvent *&aEvent, 
		DWORD *aRemainSize = NULL);

	QtResult PostEventWithOldSize(
		IQtEvent *aEvent, 
		EPriority aPri = EPRIORITY_NORMAL, 
		DWORD *aOldSize = NULL);

private:
	typedef CQtMutexThread MutexType;
	MutexType m_Mutex;
};


// inline functions
inline void CQtEventQueueBase::Reset2CurrentThreadId()
{
	m_Tid = CQtThreadManager::GetThreadSelfId();
}

inline void CQtEventQueueBase::Stop()
{
	m_bIsStopped = TRUE;
}


inline CQtEventQueueUsingMutex::CQtEventQueueUsingMutex()
{
}

inline QtResult CQtEventQueueUsingMutex::
PopPendingEventsWithoutWait(CQtEventQueueBase::EventsType &aEvents, 
							DWORD aMaxCount, DWORD *aRemainSize)
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	return PopPendingEvents(aEvents, aMaxCount, aRemainSize);
}

inline QtResult CQtEventQueueUsingMutex::
PopOnePendingEventWithoutWait(IQtEvent *&aEvent, DWORD *aRemainSize)
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	return PopOnePendingEvent(aEvent, aRemainSize);
}

#endif // !QTEVENTQUEUEBASE_H
