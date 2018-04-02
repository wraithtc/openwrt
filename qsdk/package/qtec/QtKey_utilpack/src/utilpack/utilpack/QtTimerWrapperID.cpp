
#include "QtBase.h"
#include "QtTimerWrapperID.h"
#include "QtThreadManager.h"
#include "QtThreadInterface.h"
#include "QtTimeValue.h"


//////////////////////////////////////////////////////////////////////
// class CQtTimerWrapperIDSink
//////////////////////////////////////////////////////////////////////

CQtTimerWrapperIDSink::~CQtTimerWrapperIDSink()
{
}


//////////////////////////////////////////////////////////////////////
// class CQtTimerWrapperID
//////////////////////////////////////////////////////////////////////

CQtTimerWrapperID::CQtTimerWrapperID()
	: m_bScheduled(FALSE)
	, m_pTimerQueue(NULL)
{
	// Don't get timer queue in the contruct function.
	// get timer queue in the Schedule() function.
}

CQtTimerWrapperID::~CQtTimerWrapperID()
{
	Cancel();
}

QtResult CQtTimerWrapperID::
Schedule(CQtTimerWrapperIDSink *aSink, const CQtTimeValue &aInterval, DWORD aCount)
{
	QtResult rv = QT_ERROR_NULL_POINTER;
	QT_ASSERTE(aSink);

	if (!m_pTimerQueue) {
		AQtThread* pThread = 
			CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
		if (pThread)
			m_pTimerQueue = pThread->GetTimerQueue();

		if (!m_pTimerQueue) {
			QT_ERROR_TRACE_THIS("CQtTimerWrapperID::Schedule, this thread doesn't suppport TimerQueue!");
			return rv;
		}
	}

	m_bScheduled = TRUE;
	rv = m_pTimerQueue->ScheduleTimer(this, aSink, aInterval, aCount);
	return rv;
}

QtResult CQtTimerWrapperID::Cancel()
{
	if (!m_bScheduled)
		return 0;
	m_bScheduled = FALSE;

	QtResult rv = QT_ERROR_NULL_POINTER;
	if (m_pTimerQueue)
		rv = m_pTimerQueue->CancelTimer(this);
	return rv;
}

void CQtTimerWrapperID::OnTimeout(const CQtTimeValue &, LPVOID aArg)
{
	QT_ASSERTE(m_bScheduled);
	
	CQtTimerWrapperIDSink *pSink = static_cast<CQtTimerWrapperIDSink *>(aArg);
	QT_ASSERTE(pSink);
	if (pSink)
		pSink->OnTimer(this);
}
