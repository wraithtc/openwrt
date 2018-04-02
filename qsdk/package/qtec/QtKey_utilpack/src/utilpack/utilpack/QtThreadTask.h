/*------------------------------------------------------*/
/* One thread composes one event queue                  */
/*                                                      */
/* QtThreadTask.h                                       */
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

#ifndef QTTHREADTASK_H
#define QTTHREADTASK_H

#include "QtThread.h"
#include "QtEventQueueBase.h"
#include "QtUtilClasses.h"
#include "QtObserver.h"

class CQtTimerQueueBase;
class CQtTimeValue;

template <class QueueType>
class CQtEventStopT : public IQtEvent
{
public:
	CQtEventStopT(QueueType *aQueue)
		: m_pQueue(aQueue)
	{
		QT_ASSERTE(m_pQueue);
	}

	virtual ~CQtEventStopT()
	{
	}
	
	virtual QtResult OnEventFire()
	{
		if (m_pQueue)
			m_pQueue->SetStopFlag();
		return QT_OK;
	}

	static QtResult PostStopEvent(QueueType *aQueue)
	{
		CQtEventStopT<QueueType> *pEvent = new CQtEventStopT<QueueType>(aQueue);
		if (!pEvent)
			return QT_ERROR_OUT_OF_MEMORY;
		return aQueue->GetEventQueue()->PostEvent(pEvent);
	}

private:
	QueueType *m_pQueue;
};

class CQtEventQueueUsingConditionVariable
	: public CQtEventQueueBase 
{
public:
	CQtEventQueueUsingConditionVariable();
	virtual ~CQtEventQueueUsingConditionVariable();

	// interface IQtEventQueue
	virtual QtResult PostEvent(IQtEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);

	// Pop <aMaxCount> pending events in the queue, 
	// if no events are pending, wait <aTimeout>.
	QtResult PopOrWaitPendingEvents(
		CQtEventQueueBase::EventsType &aEvents, 
		CQtTimeValue *aTimeout = NULL,
		DWORD aMaxCount = MAX_GET_ONCE);

private:
	typedef CQtMutexThread MutexType;
	MutexType m_Mutex;
	CQtConditionVariableThread m_Condition;
};

class CQtThreadTaskWithEventQueueOnly 
	: public AQtThread
	, public CQtStopFlag
{
public:
	CQtThreadTaskWithEventQueueOnly();
	virtual ~CQtThreadTaskWithEventQueueOnly();

	// interface AQtThread
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	virtual IQtEventQueue* GetEventQueue();

protected:
	CQtEventQueueUsingConditionVariable m_EventQueue;

	friend class CQtEventStopT<CQtThreadTaskWithEventQueueOnly>;
};

class CQtThreadTask : public CQtThreadTaskWithEventQueueOnly
{
	CQtThreadTask(const CQtThreadTask &);
	CQtThreadTask & operator=(const CQtThreadTask &);
public:
	CQtThreadTask();
	virtual ~CQtThreadTask();

	// interface AQtThread
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	virtual IQtTimerQueue* GetTimerQueue();

protected:
	CQtTimerQueueBase *m_pTimerQueue;
};


class CQtThreadHeartBeat : public AQtThread
{
	CQtThreadHeartBeat(const CQtThreadHeartBeat &);
	CQtThreadHeartBeat & operator=(const CQtThreadHeartBeat &);
public:
	CQtThreadHeartBeat();
	virtual ~CQtThreadHeartBeat();

	// interface AQtThread
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	virtual IQtEventQueue* GetEventQueue();
	virtual IQtTimerQueue* GetTimerQueue();

	QtResult DoHeartBeat();

protected:
	CQtEventQueueUsingMutex m_EventQueue;
	CQtTimerQueueBase *m_pTimerQueue;
};

#endif // !QTTHREADTASK_H
