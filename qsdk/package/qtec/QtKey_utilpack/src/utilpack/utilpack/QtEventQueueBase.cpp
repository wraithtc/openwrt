
#include "QtBase.h"
#include "QtEventQueueBase.h"

//////////////////////////////////////////////////////////////////////
// class CQtEventSynchronous
//////////////////////////////////////////////////////////////////////

CQtEventSynchronous::CQtEventSynchronous(IQtEvent *aEventPost, 
										 CQtEventQueueBase *aEventQueue)
	: m_pEventPost(aEventPost)
	, m_Result(QT_ERROR_NOT_AVAILABLE)
	, m_pEventQueue(aEventQueue)
	, m_bHasDestoryed(FALSE)
{
	QT_ASSERTE(m_pEventPost);
	QT_ASSERTE(m_pEventQueue);
}

CQtEventSynchronous::~CQtEventSynchronous()
{
	if (m_pEventPost)
		m_pEventPost->OnDestorySelf();
	if (!m_bHasDestoryed)
		m_SendEvent.Signal();
}

QtResult CQtEventSynchronous::OnEventFire()
{
	QtResult rv = QT_OK;
	if (m_bHasDestoryed) 
		return rv;
	
	if (m_pEventPost)
		rv = m_pEventPost->OnEventFire();
	else
		rv = QT_ERROR_NULL_POINTER;

	m_Result = rv;
	m_SendEvent.Signal();
	return rv;
}

void CQtEventSynchronous::OnDestorySelf()
{
	if (m_bHasDestoryed) {
		// delete this in the cecond time of OnDestorySelf.
		delete this;
	}
	else {
		// Don't assign <m_bHasDestoryed> in the function WaitResultAndDeleteThis().
		// Do operations on <m_bHasDestoryed> in the same thread.
		m_bHasDestoryed = TRUE;
		if (m_pEventPost) {
			m_pEventPost->OnDestorySelf();
			m_pEventPost = NULL;
		}
	}
}

QtResult CQtEventSynchronous::WaitResultAndDeleteThis()
{
	QtResult rv = m_SendEvent.Wait();
	if (QT_FAILED(rv)) {
		QT_WARNING_TRACE_THIS("CQtEventSynchronous::WaitResultAndDeleteThis,"
			" m_SendEvent.Wait() failed!");
		return rv;
	}

	rv = m_Result;
	if (m_pEventQueue)
		m_pEventQueue->PostEvent(this);
	return rv;
}


//////////////////////////////////////////////////////////////////////
// class CQtEventQueueBase
//////////////////////////////////////////////////////////////////////

#ifdef QT_WIN32
CQtTimeValue CQtEventQueueBase::s_tvReportInterval(0, 100*1000);
#else
CQtTimeValue CQtEventQueueBase::s_tvReportInterval(0, 50*1000);
#endif // QT_WIN32

CQtEventQueueBase::CQtEventQueueBase()
	: m_dwSize(0)
	, m_bIsStopped(FALSE)
{
	m_tvReportSize = CQtTimeValue::GetTimeOfDay();
}

CQtEventQueueBase::~CQtEventQueueBase()
{
	DestoryPendingEvents();	
}

void CQtEventQueueBase::DestoryPendingEvents()
{
	EventsType::iterator iter = m_Events.begin();
	for ( ; iter != m_Events.end(); ++iter)
		(*iter)->OnDestorySelf();
	m_Events.clear();
}

QtResult CQtEventQueueBase::PostEvent(IQtEvent *aEvent, EPriority aPri)
{
	QT_ASSERTE_RETURN(aEvent, QT_ERROR_INVALID_ARG);

	if (m_bIsStopped) {
		QT_WARNING_TRACE_THIS("CQtEventQueueBase::PostEvent, has been stopped.");
		aEvent->OnDestorySelf();
		return QT_ERROR_NOT_INITIALIZED;
	}

	m_Events.push_back(aEvent);
	m_dwSize++;

#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
	if (tvCur - m_tvReportSize > CQtTimeValue(3, 0)) {
		if (m_dwSize > 100)
			QT_WARNING_TRACE_THIS("CQtEventQueueBase::PostEvent,"
				" m_dwSize=" << m_dwSize << 
				" m_Tid=" << m_Tid);
		m_tvReportSize = tvCur;
	}
#endif // !QT_DISABLE_EVENT_REPORT
	
	return QT_OK;
}

QtResult CQtEventQueueBase::SendEvent(IQtEvent *aEvent)
{
	QT_ASSERTE_RETURN(aEvent, QT_ERROR_INVALID_ARG);

	if (m_bIsStopped) {
		QT_WARNING_TRACE_THIS("CQtEventQueueBase::SendEvent, has been stopped.");
		aEvent->OnDestorySelf();
		return QT_ERROR_NOT_INITIALIZED;
	}

	// if send event to the current thread, just do callbacks.
	if (CQtThreadManager::IsEqualCurrentThread(m_Tid)) {
		QtResult rv = aEvent->OnEventFire();
		aEvent->OnDestorySelf();
		return rv;
	}

	CQtEventSynchronous *pSend = new CQtEventSynchronous(aEvent, this);
	QtResult rv = PostEvent(pSend);
	if (QT_FAILED(rv))
		return rv;

	rv = pSend->WaitResultAndDeleteThis();
	return rv;
}

DWORD CQtEventQueueBase::GetPendingEventsCount()
{
	return m_dwSize;
}

#include "QtReactorBase.h"
QtResult CQtEventQueueBase::
PopPendingEvents(EventsType &aEvents, DWORD aMaxCount, DWORD *aRemainSize)
{
	QT_ASSERTE(aEvents.empty());
	QT_ASSERTE(aMaxCount > 0);
	
	DWORD dwTotal = m_dwSize;
	if (dwTotal == 0)
	{
		return QT_ERROR_NOT_FOUND;
	}
	
	if (dwTotal <= aMaxCount) {
		aEvents.swap(m_Events);
		m_dwSize = 0;
		QT_ASSERTE(m_Events.empty());
	}
	else {
		for (DWORD i = 0; i < aMaxCount; i++) {
			aEvents.push_back(m_Events.front());
			m_Events.pop_front();
			m_dwSize--;
		}
		QT_ASSERTE(!m_Events.empty());
	}

	if (aRemainSize)
		*aRemainSize = m_dwSize;
	return QT_OK;
}

QtResult CQtEventQueueBase::
PopOnePendingEvent(IQtEvent *&aEvent, DWORD *aRemainSize)
{
	QT_ASSERTE(!aEvent);
	
	if (m_dwSize == 0)
		return QT_ERROR_NOT_FOUND;

	aEvent = m_Events.front();
	m_Events.pop_front();
	m_dwSize--;

	if (aRemainSize)
		*aRemainSize = m_dwSize;
	return QT_OK;
}

QtResult CQtEventQueueBase::ProcessEvents(const EventsType &aEvents)
{
	EventsType::const_iterator iter = aEvents.begin();
	for ( ; iter != aEvents.end(); ++iter) {
		ProcessOneEvent(*iter);
	}
	return QT_OK;
}

QtResult CQtEventQueueBase::ProcessOneEvent(IQtEvent *aEvent)
{
	QT_ASSERTE_RETURN(aEvent, QT_ERROR_INVALID_ARG);

#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
#endif // !QT_DISABLE_EVENT_REPORT
	
	aEvent->OnEventFire();
	aEvent->OnDestorySelf();
	
#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvSub = CQtTimeValue::GetTimeOfDay() - tvCur;
	if (tvSub > s_tvReportInterval) {
		QT_WARNING_TRACE_THIS("CQtEventQueueBase::ProcessOneEvent, report,"
			" sec=" << tvSub.GetSec() << 
			" usec=" << tvSub.GetUsec() <<
			" aEvent=" << aEvent << 
			" m_dwSize=" << m_dwSize);
	}
#endif // !QT_DISABLE_EVENT_REPORT

	return QT_OK;
}


//////////////////////////////////////////////////////////////////////
// class CQtEventQueueUsingMutex
//////////////////////////////////////////////////////////////////////

CQtEventQueueUsingMutex::~CQtEventQueueUsingMutex()
{
}

QtResult CQtEventQueueUsingMutex::PostEvent(IQtEvent *aEvent, EPriority aPri)
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	return CQtEventQueueBase::PostEvent(aEvent, aPri);
}

#include "QtReactorBase.h"
QtResult CQtEventQueueUsingMutex::
PostEventWithOldSize(IQtEvent *aEvent, EPriority aPri, DWORD *aOldSize)
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	if (aOldSize)
		*aOldSize = m_dwSize;
	return CQtEventQueueBase::PostEvent(aEvent, aPri);
}
