
#include "QtBase.h"
#include "QtTimerQueueCalendar.h"
#include "QtThreadManager.h"

CQtTimerQueueCalendar::
CQtTimerQueueCalendar(DWORD aSlotInterval, DWORD aMaxTime, IQtEventQueue *aEq)
	: m_dwInterval(aSlotInterval)
	, m_ppSlots(NULL)
	, m_dwMaxSlotNumber(0)
	, m_dwCurrentSlot(0)
	, m_pEventQueue(aEq)
	, m_pEventSlot(NULL)
{
	QT_ASSERTE(m_dwInterval >= 10);
	if (m_dwInterval < 10)
		m_dwInterval = 10;

	if (aMaxTime >= m_dwInterval)
		m_dwMaxSlotNumber = aMaxTime / m_dwInterval - 1;
	if (m_dwMaxSlotNumber < 10)
		m_dwMaxSlotNumber = 10;

	m_ppSlots = new SlotType*[m_dwMaxSlotNumber + 1];
	::memset(m_ppSlots, 0, sizeof(SlotType *) * (m_dwMaxSlotNumber+1));

	if (!m_pEventQueue) {
		m_pEventQueue = CQtThreadManager::Instance()->GetThreadEventQueue(CQtThreadManager::TT_CURRENT);
		QT_ASSERTE(m_pEventQueue);
	}
}

CQtTimerQueueCalendar::~CQtTimerQueueCalendar()
{
	SlotType *pFirst = m_pEventSlot;
	while (pFirst) {
		SlotType *pTmp = pFirst;
		pFirst = pTmp->m_pNext;
		DeleteSlot_i(pTmp);
	}
	
	for (DWORD i = 0; i <= m_dwMaxSlotNumber; i++) {
		// TODO: delete all slots.
		SlotType *pFirst = m_ppSlots[i];
		while (pFirst) {
			SlotType *pTmp = pFirst;
			pFirst = pTmp->m_pNext;
			DeleteSlot_i(pTmp);
		}
	}
	delete []m_ppSlots;
}

QtResult CQtTimerQueueCalendar::
ScheduleTimer(IQtTimerHandler *aEh, LPVOID aToken, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	m_Est.EnsureSingleThread();
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);

	// find slot first.
	BOOL bFound = FALSE;
	SlotType *pFind = RemoveUniqueHandler_i(aEh);
	if (!pFind) {
		// alloc slot if not found.
		ValueType valueNew(
			aEh, aToken, aInterval, 
			aCount > 0 ? aCount : (DWORD)-1);
		pFind = NewSlot_i(valueNew);
		if (!pFind)
			return QT_ERROR_OUT_OF_MEMORY;
	}
	else {
		QT_ASSERTE(pFind->m_Value.m_pHanler == aEh);
		pFind->m_Value.m_pToken = aToken;
		pFind->m_Value.m_tvInterval = aInterval;
		pFind->m_Value.m_dwCount = aCount > 0 ? aCount : (DWORD)-1;
		pFind->m_pNext = NULL;
		bFound = TRUE;
	}
	
	if (aInterval == CQtTimeValue::s_tvZero) {
		// if interval is 0, use event queue instead.
		QT_ASSERTE(aCount == 1);
		BOOL bNeedPost = m_pEventSlot ? FALSE : TRUE;
		pFind->m_pNext = m_pEventSlot;
		m_pEventSlot = pFind;
		QtResult rv = bFound ? QT_ERROR_FOUND : QT_OK;
		if (bNeedPost) {
			rv = m_pEventQueue->PostEvent(this);
			if (QT_FAILED(rv)) {
				m_pEventSlot = m_pEventSlot->m_pNext;
				DeleteSlot_i(pFind);
			}
		}
		return rv;
	}

	InsertUnique_i(aInterval, pFind);
	return bFound ? QT_ERROR_FOUND : QT_OK;
}

void CQtTimerQueueCalendar::InsertUnique_i(const CQtTimeValue &aInterval, SlotType *aInsert)
{
	QT_ASSERTE(aInsert);

	DWORD dwMs = aInterval.GetTotalInMsec();
	DWORD dwDistance = dwMs / m_dwInterval;
	if (dwMs % m_dwInterval)
		dwDistance++;

	// budingc fixed bug #137328 at 12/13/2004, 
	// m_dwMaxSlotNumber - 1 to avoid OnTimer infinite times.
	if (dwDistance > m_dwMaxSlotNumber - 1) {
		QT_ERROR_TRACE_THIS("CQtTimerQueueCalendar::InsertUnique_i, exceed max interval."
			" interval_s=" << aInterval.GetSec() << 
			" interval_us=" << aInterval.GetUsec() << 
			" dwDistance=" << dwDistance << 
			" m_dwMaxSlotNumber=" << m_dwMaxSlotNumber);
		QT_ASSERTE(FALSE);
		dwDistance = m_dwMaxSlotNumber;
	}

	if (dwDistance > m_dwMaxSlotNumber - m_dwCurrentSlot) {
		dwDistance -= m_dwMaxSlotNumber - m_dwCurrentSlot;
	}
	else {
		dwDistance += m_dwCurrentSlot;
	}
	aInsert->m_pNext = m_ppSlots[dwDistance];
	m_ppSlots[dwDistance] = aInsert;
	m_Hanlers[aInsert->m_Value.m_pHanler] = dwDistance;
}

QtResult CQtTimerQueueCalendar::CancelTimer(IQtTimerHandler *aEh)
{
	m_Est.EnsureSingleThread();
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);

	SlotType *pFind = RemoveUniqueHandler_i(aEh);
	if (pFind) {
#ifdef QT_DEBUG
		HanlersType::size_type nErase = 
#endif //QT_DEBUG
			m_Hanlers.erase(pFind->m_Value.m_pHanler);
#ifdef QT_DEBUG
		QT_ASSERTE(nErase == 1);
#endif

		DeleteSlot_i(pFind);
		return QT_OK;
	}
	else {
		return QT_ERROR_NOT_FOUND;
	}
}

void CQtTimerQueueCalendar::TimerTick()
{
	m_Est.EnsureSingleThread();

	// budingc fixed bug #137328 at 12/13/2004, 
	// upper layer may cancel other timer which is in current slot.
	DWORD dwCur = m_dwCurrentSlot;
	SlotType *pFirst = m_ppSlots[dwCur];
//	m_ppSlots[dwCur] = NULL;
	if (pFirst)
		m_ppSlots[dwCur] = pFirst->m_pNext;

	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
	while (pFirst) {
		QT_ASSERTE(pFirst->m_Value.m_tvInterval > CQtTimeValue::s_tvZero);
		HandlerType handlerOn = pFirst->m_Value.m_pHanler;
		LPVOID pToken = pFirst->m_Value.m_pToken;
		
		if (--pFirst->m_Value.m_dwCount > 0) {
			// Don't use pTmp->m_pNext due to modified by InsertUnique_i().
			SlotType *pTmp = pFirst;
//			pFirst = pFirst->m_pNext;
			InsertUnique_i(pTmp->m_Value.m_tvInterval, pTmp);
		}
		else {
#ifdef QT_DEBUG
			HanlersType::size_type nErase = 
#endif //QT_DEBUG
				m_Hanlers.erase(pFirst->m_Value.m_pHanler);
#ifdef QT_DEBUG
			QT_ASSERTE(nErase == 1);
#endif
			
			SlotType *pTmp = pFirst;
//			pFirst = pFirst->m_pNext;
			DeleteSlot_i(pTmp);
		}

		handlerOn->OnTimeout(tvCur, pToken);

		pFirst = m_ppSlots[dwCur];
		if (pFirst)
			m_ppSlots[dwCur] = pFirst->m_pNext;
	}

	// advance <m_dwCurrentSlot> after process timer callback.
	if (m_dwCurrentSlot == m_dwMaxSlotNumber) {
		m_dwCurrentSlot = 0;
	}
	else
		m_dwCurrentSlot++;
}

QtResult CQtTimerQueueCalendar::OnEventFire()
{
	// budingc fixed bug #213676 at 18/07/2006.
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
	SlotType *pFirst = m_pEventSlot;
//	m_pEventSlot = NULL;
	if (m_pEventSlot)
		m_pEventSlot = m_pEventSlot->m_pNext;
	
	while (pFirst) {
		SlotType *pTmp = pFirst;
		QT_ASSERTE(pTmp->m_Value.m_tvInterval == CQtTimeValue::s_tvZero);
		pTmp->m_Value.m_pHanler->OnTimeout(tvCur, pTmp->m_Value.m_pToken);

		//pFirst = pTmp->m_pNext;
		DeleteSlot_i(pTmp);

		pFirst = m_pEventSlot;
		if (m_pEventSlot)
			m_pEventSlot = m_pEventSlot->m_pNext;
	}
	return QT_OK;
}

void CQtTimerQueueCalendar::OnDestorySelf()
{
	// do nothing to avoid be deleted.
}

CQtTimerQueueCalendar::SlotType* CQtTimerQueueCalendar::
RemoveUniqueHandler_i(const HandlerType &aHanler)
{
	HanlersType::iterator iter = m_Hanlers.find(aHanler);
	if (iter == m_Hanlers.end()) {
		// remove handler in event slot.
		SlotType *pMove = m_pEventSlot;
		SlotType *pPreTmp = NULL;
		while (pMove) {
			if (pMove->m_Value.m_pHanler == aHanler) {
				if (pPreTmp)
					pPreTmp->m_pNext = pMove->m_pNext;
				else
					m_pEventSlot = pMove->m_pNext;
				DeleteSlot_i(pMove);
				break;
			}
			else {
				pPreTmp = pMove;
				pMove = pMove->m_pNext;
			}
		}

		return NULL;
	}

	DWORD dwIndex = (*iter).second;
	QT_ASSERTE(dwIndex <= m_dwMaxSlotNumber);

	SlotType *pRet = RemoveUniqueSlot_i(m_ppSlots[dwIndex], aHanler);
//	QT_ASSERTE(pRet);
	return pRet;
}

CQtTimerQueueCalendar::SlotType* CQtTimerQueueCalendar::
RemoveUniqueSlot_i(SlotType *&aFirst, const HandlerType &aHanler)
{
	if (aFirst) {
		if (aFirst->m_Value.m_pHanler == aHanler) {
			SlotType *pRet = aFirst;
			aFirst = aFirst->m_pNext;
			return pRet;
		}

		SlotType *pCur = aFirst;
		SlotType *pNext = pCur->m_pNext;
		while (pNext) {
			if (pNext->m_Value.m_pHanler == aHanler) {
				pCur->m_pNext = pNext->m_pNext;
				return pNext;
			}
			else {
				pCur = pNext;
				pNext = pNext->m_pNext;
			}
		}
	}
	return NULL;
}
