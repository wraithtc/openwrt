
#include "QtBase.h"
#include "QtTimerQueueBase.h"
#include "QtReactorInterface.h"
#include "QtObserver.h"

CQtTimerQueueBase::CQtTimerQueueBase(IQtObserver *aObserver)
	: m_pObserver(aObserver)
{
}

CQtTimerQueueBase::~CQtTimerQueueBase()
{
}

int CQtTimerQueueBase::CheckExpire(CQtTimeValue *aRemainTime)
{
	int nCout = 0;
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();

	for ( ; ; ) {
		IQtTimerHandler *pEh = NULL;
		LPVOID pToken = NULL;
		{
			CQtTimeValue tvEarliest;
			CQtMutexGuardT<MutexType> theGuard(m_Mutex);
			int nRet = GetEarliestTime_l(tvEarliest);
			if (nRet == -1) {
				if (aRemainTime)
					*aRemainTime = CQtTimeValue::s_tvMax;
				break;
			}
			else if (tvEarliest > tvCur) {
				if (aRemainTime)
					*aRemainTime = tvEarliest - tvCur;
				break;
			}

			CNode ndFirst;
			nRet = PopFirstNode_l(ndFirst);
			QT_ASSERTE(nRet == 0);

			pEh = ndFirst.m_pEh;
			pToken = ndFirst.m_pToken;
			if (ndFirst.m_dwCount != (DWORD)-1)
				ndFirst.m_dwCount--;

			if (ndFirst.m_dwCount > 0 && ndFirst.m_tvInterval > CQtTimeValue::s_tvZero) {
				do {
					ndFirst.m_tvExpired += ndFirst.m_tvInterval;
				}
				while (ndFirst.m_tvExpired <= tvCur);

				RePushNode_l(ndFirst);
			}
		}

		QT_ASSERTE(pEh);
		pEh->OnTimeout(tvCur, pToken);
		nCout++;
	}

	return nCout;
}

QtResult CQtTimerQueueBase::
ScheduleTimer(IQtTimerHandler *aEh, LPVOID aToken, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);
	QT_ASSERTE_RETURN(aInterval > CQtTimeValue::s_tvZero || aCount == 1, QT_ERROR_INVALID_ARG);
	
	int nRet;
	BOOL bNeedNotify = FALSE;
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		CQtTimeValue tvTmp;
		if (m_pObserver)
			bNeedNotify = GetEarliestTime_l(tvTmp) == -1 ? TRUE : FALSE;
		
		CNode ndNew(aEh, aToken);
		ndNew.m_tvInterval = aInterval;
		ndNew.m_tvExpired = CQtTimeValue::GetTimeOfDay() + aInterval;
		if (aCount > 0)
			ndNew.m_dwCount = aCount;
		else
			ndNew.m_dwCount = (DWORD)-1;
		
		// PushNode_l() will check the same IQtTimerHandler in the queue 
		nRet = PushNode_l(ndNew);
	}

	if (bNeedNotify) {
		QT_ASSERTE(m_pObserver);
//		QT_INFO_TRACE_THIS("CQtTimerQueueBase::ScheduleTimer, time queue is empty, Notify it.");
		m_pObserver->OnObserve("TimerQueue");
	}

	// Timer queue is in the own thread, need not notify it.
	if (nRet == 0)
		return QT_OK;
	else if (nRet == 1)
		return QT_ERROR_FOUND;
	else
		return QT_ERROR_FAILURE;
}

QtResult CQtTimerQueueBase::CancelTimer(IQtTimerHandler *aEh)
{
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	int nRet = EraseNode_l(aEh);
	if (nRet == 0)
		return QT_OK;
	else if (nRet == 1)
		return QT_ERROR_NOT_FOUND;
	else
		return QT_ERROR_FAILURE;
}

CQtTimeValue CQtTimerQueueBase::GetEarliestTime() 
{
	CQtTimeValue tvRet;
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	int nRet = GetEarliestTime_l(tvRet);
	if (nRet == 0)
		return tvRet;
	else
		return CQtTimeValue::s_tvMax;
}

