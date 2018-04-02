#include "QtBase.h"
#include "QtTimerWrapperID.h"
#include "QtTimerQueueCalendar.h"
#include "QtConditionVariable.h"

#include "QtTimerPreciseWrapperID.h"

//////////////////////////////////////////////////////////////////////
// class CQtTimerPreciseManager
//////////////////////////////////////////////////////////////////////
static CQtTimerPreciseManager* s_pPreciseTimerMgr = NULL;

CQtTimerPreciseManager::CQtTimerPreciseManager()
{
	m_pTimerQueue = new CQtTimerQueueCalendar(10,1000*60*60*2,NULL);

	QT_ASSERTE( m_pTimerQueue );

	//call parent function,delele self by thread manager since register self.
	AQtThread::Create(CQtThreadManager::TT_TIMER, CQtThreadManager::TF_JOINABLE);
}

CQtTimerPreciseManager::~CQtTimerPreciseManager()
{
	if( m_pTimerQueue )
	{
		delete m_pTimerQueue;
		m_pTimerQueue = NULL;
	}
}

CQtTimerPreciseManager* CQtTimerPreciseManager::Instance()
{
	if( !s_pPreciseTimerMgr )
		s_pPreciseTimerMgr = new CQtTimerPreciseManager();

	return s_pPreciseTimerMgr;
}

QtResult CQtTimerPreciseManager::Schedule(
		CQtTimerPreciseWrapperID* aTimerId,
		CQtTimerPreciseWrapperIDSink* aSink,
		const CQtTimeValue &aInterval,
		DWORD aCount)
{
	QT_ASSERTE( aTimerId );
	if( m_pTimerQueue )
	{
		CQtMutexGuardT<CQtMutexThread> theGuard(m_Mutex);
		m_pTimerQueue->ScheduleTimer(aTimerId,aSink,aInterval,aCount);
	}

	return QT_OK;
}

QtResult CQtTimerPreciseManager::Cancel(CQtTimerPreciseWrapperID* aTimerId)
{
	QtResult rv = QT_ERROR_NULL_POINTER;
	if (aTimerId && m_pTimerQueue)
	{
		CQtMutexGuardT<CQtMutexThread> theGuard(m_Mutex);

		rv = m_pTimerQueue->CancelTimer( aTimerId );
	}

	return rv;
}

void CQtTimerPreciseManager::OnThreadInit()
{
	m_pTimerQueue->m_Est.Reset2CurrentThreadId();
	
	CQtStopFlag::m_Est.Reset2CurrentThreadId();
	SetStartFlag();
}

void CQtTimerPreciseManager::OnThreadRun()
{
	CQtEventThread Event;
	CQtTimeValue tv(0,10000);
	//HANDLE hSemaphore = ::CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
	while (!IsFlagStopped()) 
	{
		Event.Wait(&tv);
		//::WaitForSingleObject(hSemaphore, 10);
		//Sleep( 10 );
		CQtMutexGuardT<CQtMutexThread> theGuard(m_Mutex);

		m_pTimerQueue->TimerTick();
	}
	//CloseHandle( hSemaphore );
}

QtResult CQtTimerPreciseManager::Stop(CQtTimeValue* aTimeout )
{
	SetStopFlag();
	return QT_OK;
}


//////////////////////////////////////////////////////////////////////
// class CQtTimerPreciseWrapperID
//////////////////////////////////////////////////////////////////////
CQtTimerPreciseWrapperID::CQtTimerPreciseWrapperID()
	: m_bScheduled(FALSE)
	, m_TType(CQtThreadManager::TT_UNKNOWN)
	, m_pThread( NULL )
	, m_pEventQueue( NULL)
{
	CQtTimerPreciseManager::Instance();
	QT_ASSERTE( s_pPreciseTimerMgr );
}

CQtTimerPreciseWrapperID::~CQtTimerPreciseWrapperID()
{
	Cancel();
}

QtResult CQtTimerPreciseWrapperID::Schedule(
		CQtTimerPreciseWrapperIDSink* aSink,
		const CQtTimeValue &aInterval,
		DWORD aCount,
		CQtThreadManager::TType aTType)
{
	if( aTType == CQtThreadManager::TT_CURRENT ||
		aTType == CQtThreadManager::TT_TIMER)
		return QT_ERROR_INVALID_ARG;
	
	m_TType = aTType;
	m_pThread = CQtThreadManager::Instance()->GetThread(m_TType);
	QT_ASSERTE( m_pThread );
	if( !m_pThread )
		return QT_ERROR_NULL_POINTER;

	m_pEventQueue = m_pThread->GetEventQueue();
	QT_ASSERTE( m_pEventQueue );
	if( !m_pEventQueue )
		return QT_ERROR_NULL_POINTER;

	if( !s_pPreciseTimerMgr )
		return QT_ERROR_NULL_POINTER;

	s_pPreciseTimerMgr->Schedule(this,aSink,aInterval,aCount);

	m_bScheduled = TRUE;

	return QT_OK;
}

QtResult CQtTimerPreciseWrapperID::Cancel()
{
	if (!m_bScheduled)
		return QT_ERROR_FAILURE;

	if( !s_pPreciseTimerMgr )
		return QT_ERROR_NULL_POINTER;

	s_pPreciseTimerMgr->Cancel(this);

	m_bScheduled = FALSE;
	
	return QT_OK;
}

void CQtTimerPreciseWrapperID::OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg)
{
	QT_ASSERTE(m_bScheduled);
	if( !m_bScheduled)
		return;
	
	CQtTimerPreciseWrapperIDSink *pSink = static_cast<CQtTimerPreciseWrapperIDSink *>(aArg);
	QT_ASSERTE(pSink);
	if( pSink && m_pEventQueue )
	{
		CEventTimer *pEvent = new CEventTimer( pSink,this );
		m_pEventQueue->PostEvent(pEvent);
	}
}

//////////////////////////////////////////////////////////////////////
// class CEventTimer
//////////////////////////////////////////////////////////////////////
CEventTimer::CEventTimer(CQtTimerPreciseWrapperIDSink *pSink, CQtTimerPreciseWrapperID* pTimerId)
	:m_pSink( pSink )
	,m_pTimerId( pTimerId )
{
}

CEventTimer::~CEventTimer()
{
}

QtResult CEventTimer::OnEventFire()
{
	if( m_pTimerId->GetStatus() )
		m_pSink->OnTimer( m_pTimerId );

	return QT_OK;
}
