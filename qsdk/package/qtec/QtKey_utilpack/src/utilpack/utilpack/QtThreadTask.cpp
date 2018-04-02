
#include "QtBase.h"
#include "QtThreadTask.h"
#include "QtTimerQueueOrderedList.h"

//////////////////////////////////////////////////////////////////////
// class CQtThreadTaskWithEventQueueOnly
//////////////////////////////////////////////////////////////////////

CQtThreadTaskWithEventQueueOnly::CQtThreadTaskWithEventQueueOnly()
{
}

CQtThreadTaskWithEventQueueOnly::~CQtThreadTaskWithEventQueueOnly()
{
}

void CQtThreadTaskWithEventQueueOnly::OnThreadInit()
{
	m_EventQueue.Reset2CurrentThreadId();

	CQtStopFlag::m_Est.Reset2CurrentThreadId();
	SetStartFlag();
}

void CQtThreadTaskWithEventQueueOnly::OnThreadRun()
{
	QT_INFO_TRACE_THIS("CQtThreadTaskWithEventQueueOnly::OnThreadRun, Begin.");

	while (!IsFlagStopped()) {
		CQtEventQueueBase::EventsType listEvents;
		QtResult rv = m_EventQueue.PopOrWaitPendingEvents(listEvents, NULL, (DWORD)-1);

		if (QT_SUCCEEDED(rv))
			m_EventQueue.ProcessEvents(listEvents);
	}

	m_EventQueue.DestoryPendingEvents();
	QT_INFO_TRACE_THIS("CQtThreadTaskWithEventQueueOnly::OnThreadRun, End.");
}

IQtEventQueue* CQtThreadTaskWithEventQueueOnly::GetEventQueue()
{
	return &m_EventQueue;
}

QtResult CQtThreadTaskWithEventQueueOnly::Stop(CQtTimeValue* aTimeout)
{
	QT_INFO_TRACE_THIS("CQtThreadTaskWithEventQueueOnly::Stop");

	QtResult rv = CQtEventStopT<CQtThreadTaskWithEventQueueOnly>::PostStopEvent(this);

	// stop event queue after post stop event.
	m_EventQueue.Stop();
	return rv;
}


//////////////////////////////////////////////////////////////////////
// class CQtThreadTask
//////////////////////////////////////////////////////////////////////

CQtThreadTask::CQtThreadTask()
	: m_pTimerQueue(NULL)
{
}

CQtThreadTask::~CQtThreadTask()
{
	delete m_pTimerQueue;
}

void CQtThreadTask::OnThreadInit()
{
	// have to new timerqueue in the task thread.
	QT_ASSERTE(!m_pTimerQueue);
	m_pTimerQueue = new CQtTimerQueueOrderedList(NULL);
	QT_ASSERTE(m_pTimerQueue);

	CQtThreadTaskWithEventQueueOnly::OnThreadInit();
}

void CQtThreadTask::OnThreadRun()
{
	QT_INFO_TRACE_THIS("CQtThreadTask::OnThreadRun, Begin.");

	while (!IsFlagStopped()) {
		// improve the performance.
#if 0
		// CheckExpire before Wait.
		m_pTimerQueue->CheckExpire();

		CQtTimeValue tvTimeout = CQtTimeValue::s_tvZero;
		CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
		CQtTimeValue tvEarliest = m_pTimerQueue->GetEarliestTime();
		if (tvCur < tvEarliest) {
			if (tvEarliest != CQtTimeValue::s_tvMax)
				tvTimeout = tvEarliest - tvCur;
			else
				tvTimeout = CQtTimeValue::s_tvMax;
		}
#else
		CQtTimeValue tvTimeout(CQtTimeValue::s_tvMax);
		if (m_pTimerQueue) {
			// process timer prior to wait event.
			m_pTimerQueue->CheckExpire(&tvTimeout);
		}
#endif

		CQtTimeValue *pTvPara;
		if (tvTimeout == CQtTimeValue::s_tvMax)
			pTvPara = NULL;
		else
			pTvPara = &tvTimeout;

		CQtEventQueueBase::EventsType listEvents;
		QtResult rv = m_EventQueue.PopOrWaitPendingEvents(listEvents, pTvPara);

		// CheckExpire after Wait.
//		m_pTimerQueue->CheckExpire();

		if (QT_SUCCEEDED(rv))
			m_EventQueue.ProcessEvents(listEvents);
	}

	m_EventQueue.DestoryPendingEvents();
	QT_INFO_TRACE_THIS("CQtThreadTask::OnThreadRun, End.");
}

IQtTimerQueue* CQtThreadTask::GetTimerQueue()
{
	return m_pTimerQueue;
}

//////////////////////////////////////////////////////////////////////
// class CQtEventQueueUsingConditionVariable
//////////////////////////////////////////////////////////////////////

CQtEventQueueUsingConditionVariable::CQtEventQueueUsingConditionVariable()
	: m_Condition(m_Mutex)
{
}

CQtEventQueueUsingConditionVariable::~CQtEventQueueUsingConditionVariable()
{
}

QtResult CQtEventQueueUsingConditionVariable::
PostEvent(IQtEvent *aEvent, EPriority aPri)
{
	// Don't hold the mutex when signaling the condition variable.
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		QtResult rv = CQtEventQueueBase::PostEvent(aEvent, aPri);
		if (QT_FAILED(rv))
			return rv;
	}

	// Don't care the error if Signal() failed.
	m_Condition.Signal();
	return QT_OK;
}

QtResult CQtEventQueueUsingConditionVariable::
PopOrWaitPendingEvents(CQtEventQueueBase::EventsType &aEvents, 
					   CQtTimeValue *aTimeout, DWORD aMaxCount)
{
	QtResult rv;
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	if (m_Events.empty()) {
		rv = m_Condition.Wait(aTimeout);
		if (QT_FAILED(rv) && rv != QT_ERROR_TIMEOUT) {
			QT_ERROR_TRACE_THIS("CQtEventQueueUsingConditionVariable::PopOrWaitPendingEvents,"
				"m_Events is not empty. nSize=" << m_dwSize << " rv=" << rv);
		//	return rv;
		}
	}
	
	return PopPendingEvents(aEvents, aMaxCount);
}


//////////////////////////////////////////////////////////////////////
// class CQtThreadHeartBeat
//////////////////////////////////////////////////////////////////////

CQtThreadHeartBeat::CQtThreadHeartBeat()
	: m_pTimerQueue(NULL)
{
}

CQtThreadHeartBeat::~CQtThreadHeartBeat()
{
	delete m_pTimerQueue;
}

void CQtThreadHeartBeat::OnThreadInit()
{
	// have to new timerqueue in the task thread.
	QT_ASSERTE(!m_pTimerQueue);
	m_pTimerQueue = new CQtTimerQueueOrderedList(NULL);
	QT_ASSERTE(m_pTimerQueue);

	m_EventQueue.Reset2CurrentThreadId();
}

IQtEventQueue* CQtThreadHeartBeat::GetEventQueue()
{
	return &m_EventQueue;
}

IQtTimerQueue* CQtThreadHeartBeat::GetTimerQueue()
{
	return m_pTimerQueue;
}

void CQtThreadHeartBeat::OnThreadRun()
{
	DoHeartBeat();
}

QtResult CQtThreadHeartBeat::Stop(CQtTimeValue* aTimeout)
{
	return QT_OK;
}

QtResult CQtThreadHeartBeat::DoHeartBeat()
{
	if (m_pTimerQueue)
		m_pTimerQueue->CheckExpire();

	CQtEventQueueBase::EventsType listEvents;
	QtResult rv = m_EventQueue.PopPendingEventsWithoutWait(listEvents, (DWORD)-1);
	if (QT_SUCCEEDED(rv))
		m_EventQueue.ProcessEvents(listEvents);

	return QT_OK;
}
