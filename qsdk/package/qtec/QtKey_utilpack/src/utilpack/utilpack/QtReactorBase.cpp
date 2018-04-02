
#include "QtBase.h"
#include "QtReactorBase.h"
#include "QtTimerQueueOrderedList.h"

#ifndef QT_WIN32
  #ifndef QT_MACOS
    #include <sys/resource.h>
  #endif
#endif // !QT_WIN32

//////////////////////////////////////////////////////////////////////
// class CQtEventHandlerRepository
//////////////////////////////////////////////////////////////////////

CQtEventHandlerRepository::CQtEventHandlerRepository()
#ifndef QT_WIN32
	: m_pHandlers(NULL)
	, m_nMaxHandler(0)
#endif // !QT_WIN32
{
}

CQtEventHandlerRepository::~CQtEventHandlerRepository()
{
	Close();
}

QtResult CQtEventHandlerRepository::Open()
{
#ifdef QT_WIN32
	QT_ASSERTE_RETURN(m_Handlers.empty(), QT_ERROR_ALREADY_INITIALIZED);

#else
	QT_ASSERTE_RETURN(!m_pHandlers, QT_ERROR_ALREADY_INITIALIZED);

#ifdef QT_PORT_CLIENT
	QtResult rv = SetRlimit(RLIMIT_NOFILE, 512, m_nMaxHandler);
#else
	QtResult rv = SetRlimit(RLIMIT_NOFILE, 8192, m_nMaxHandler);
#endif // 
	if (QT_FAILED(rv))
		return rv;

	m_pHandlers = new CElement[m_nMaxHandler];
	if (!m_pHandlers)
		return QT_ERROR_OUT_OF_MEMORY;
#endif // QT_WIN32

	return QT_OK;
}

QtResult CQtEventHandlerRepository::Close()
{
#ifdef QT_WIN32
	m_Handlers.clear();
#else
	if (m_pHandlers) {
		delete []m_pHandlers;
		m_pHandlers = NULL;
	}
	m_nMaxHandler = 0;
#endif // QT_WIN32
	return QT_OK;
}

#ifndef QT_WIN32

#if defined (QT_SOLARIS) || defined (QT_MACOS)
  typedef int __rlimit_resource_t;
#endif // QT_SOLARIS || QT_MACOS

QtResult CQtEventHandlerRepository::SetRlimit(int aResource, int aMaxNum, int &aActualNum)
{
	rlimit rlCur;
	::memset(&rlCur, 0, sizeof(rlCur));
	int nRet = ::getrlimit((__rlimit_resource_t)aResource, &rlCur);
	if (nRet == -1 || rlCur.rlim_cur == RLIM_INFINITY) {
		QT_ERROR_TRACE("CQtEventHandlerRepository::SetRlimit, getrlimit() failed! err=" << errno);
		return QT_ERROR_UNEXPECTED;
	}
	
	aActualNum = aMaxNum;
	if (aActualNum > static_cast<int>(rlCur.rlim_cur)) {
		rlimit rlNew;
		::memset(&rlNew, 0, sizeof(rlNew));
		rlNew.rlim_cur = aActualNum;
		rlNew.rlim_max = aActualNum;
		nRet = ::setrlimit((__rlimit_resource_t)aResource, &rlNew);
		if (nRet == -1) {
			if (errno == EPERM) {
				QT_WARNING_TRACE("CQtEventHandlerRepository::SetRlimit, setrlimit() failed. "
					"you should use superuser to setrlimit(RLIMIT_NOFILE)!");
				aActualNum = rlCur.rlim_cur;
			}
			else {
				QT_WARNING_TRACE("CQtEventHandlerRepository::SetRlimit, setrlimit() failed! err=" << errno);
				return QT_ERROR_UNEXPECTED;
			}
		}
	}
	else
		aActualNum = rlCur.rlim_cur;
	
	return QT_OK;
}
#endif // !QT_WIN32

inline void FdSet_s(fd_set &aFsRead, fd_set &aFsWrite, fd_set &aFsException, 
					CQtEventHandlerRepository::CElement &aEleGet, 
					int &aMaxFd)
{
	int nSocket = (int)aEleGet.m_pEh->GetHandle();
	if (nSocket > aMaxFd)
		aMaxFd = nSocket;

	// READ, ACCEPT, and CONNECT flag will place the handle in the read set.
	if (QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::READ_MASK) || 
		QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::ACCEPT_MASK) || 
		QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::CONNECT_MASK))
	{
		FD_SET(nSocket, &aFsRead);
	}
	// WRITE and CONNECT flag will place the handle in the write set.
	if (QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::WRITE_MASK) || 
		QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::CONNECT_MASK))
	{
		FD_SET(nSocket, &aFsWrite);
	}
#ifdef QT_WIN32
	if (QT_BIT_ENABLED(aEleGet.m_Mask, AQtEventHandler::CONNECT_MASK))
	{
		FD_SET(nSocket, &aFsException);
	}
#endif // QT_WIN32
}

int CQtEventHandlerRepository::
FillFdSets(fd_set &aFsRead, fd_set &aFsWrite, fd_set &aFsException)
{
	int nMaxFd = -1;
#ifdef QT_WIN32
	HandlersType::iterator iter = m_Handlers.begin();
	for ( ; iter != m_Handlers.end(); ++iter) {
		CElement &eleGet = (*iter).second;
		FdSet_s(aFsRead, aFsWrite, aFsException, eleGet, nMaxFd);
	}
#else
	for (int i = 0; i < m_nMaxHandler; i++) {
		CElement &eleGet = m_pHandlers[i];
		if (!eleGet.IsCleared())
			FdSet_s(aFsRead, aFsWrite, aFsException, eleGet, nMaxFd);
	}
#endif // QT_WIN32
	return nMaxFd;
}


//////////////////////////////////////////////////////////////////////
// class CQtReactorBase
//////////////////////////////////////////////////////////////////////

CQtReactorBase::CQtReactorBase(PROPERTY aProperty)
	: IQtReactor(aProperty)
	, m_pTimerQueue(NULL)
	, m_bNotifyFailed(FALSE)
{
}

CQtReactorBase::~CQtReactorBase()
{
	// needn't do Close() because the inherited class will do it
//	Close();
}

QtResult CQtReactorBase::Open()
{
	m_Est.Reset2CurrentThreadId();
	CQtEventQueueUsingMutex::Reset2CurrentThreadId();
	CQtStopFlag::m_Est.Reset2CurrentThreadId();
	
	// check whether inheried class instanced the timer queue.
	if (!m_pTimerQueue) {
		m_pTimerQueue = new CQtTimerQueueOrderedList(NULL);
		if (!m_pTimerQueue)
			return QT_ERROR_OUT_OF_MEMORY;
	}

	QtResult rv = m_EhRepository.Open();
	if (QT_FAILED(rv))
		return rv;

	return rv;
}

QtResult CQtReactorBase::
RegisterHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	// FIXME TODO: Register handler after OnClose!
	
	m_Est.EnsureSingleThread();
	QtResult rv;
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);

	BOOL IsUDP = aMask & AQtEventHandler::UDP_LINK_MASK;
	AQtEventHandler::MASK maskNew = aMask & AQtEventHandler::ALL_EVENTS_MASK;
/*
	if(IsUDP){
		maskNew |= AQtEventHandler::UDP_LINK_MASK;
		QT_INFO_TRACE_THIS("CQtReactorBase::RegisterHandler is UDP socket mask = " << maskNew);
	}
*/
	if (maskNew == AQtEventHandler::NULL_MASK) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::RegisterHandler, NULL_MASK. aMask=" << aMask);
		return QT_ERROR_INVALID_ARG;
	}

	CQtEventHandlerRepository::CElement eleFind;
	QT_HANDLE fdNew = aEh->GetHandle();
	rv = m_EhRepository.Find(fdNew, eleFind);
	if (maskNew == eleFind.m_Mask && aEh == eleFind.m_pEh) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::RegisterHandler, mask is equal."
			" aEh=" << aEh <<
			" aMask=" << aMask <<
			" fdNew=" << fdNew <<
			" rv=" << rv);
		return QT_OK;
	}

	if (eleFind.IsCleared()) {
		rv = OnHandleRegister(fdNew, IsUDP ? maskNew | AQtEventHandler::UDP_LINK_MASK : maskNew, aEh);

		// needn't remove handle when OnHandleRegister() failed
		// because the handle didn't be inserted at all
		if (QT_FAILED(rv))
			return rv;
	}

	CQtEventHandlerRepository::CElement eleNew(aEh, maskNew);
	rv = m_EhRepository.Bind(fdNew, eleNew);
	return rv;
}

QtResult CQtReactorBase::
RemoveHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	m_Est.EnsureSingleThread();
	QtResult rv;
	QT_ASSERTE_RETURN(aEh, QT_ERROR_INVALID_ARG);
	
	AQtEventHandler::MASK maskNew = aMask & AQtEventHandler::ALL_EVENTS_MASK;
	if (maskNew == AQtEventHandler::NULL_MASK) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::RemoveHandler, NULL_MASK. aMask=" << aMask);
		return QT_ERROR_INVALID_ARG;
	}

	CQtEventHandlerRepository::CElement eleFind;
	QT_HANDLE fdNew = aEh->GetHandle();
	rv = m_EhRepository.Find(fdNew, eleFind);
	if (QT_FAILED(rv)) {
/* the trace has no useful for us and too much in the trace file, disable it by Victor Mar 21 2008
		QT_WARNING_TRACE_THIS("CQtReactorBase::RemoveHandler, handle not registed."
			" aEh=" << aEh <<
			" aMask=" << aMask <<
			" fdNew=" << fdNew <<
			" rv=" << rv);
*/
		return rv;
	}

	rv = RemoveHandleWithoutFinding_i(fdNew, eleFind, maskNew);
	return rv;
}

QtResult CQtReactorBase::Close()
{
	if (m_pTimerQueue) {
		// I am sorry to comment it because PostMessage(WM_QUIT) will fail if
		// in atexit() route.
//		m_Est.EnsureSingleThread();
		delete m_pTimerQueue;
		m_pTimerQueue = NULL;
	}
	m_EhRepository.Close();
	CQtEventQueueBase::DestoryPendingEvents();
	return QT_OK;
}

QtResult CQtReactorBase::
ScheduleTimer(IQtTimerHandler *aTh, LPVOID aArg, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	m_Est.EnsureSingleThread();
	if (!m_pTimerQueue) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::ScheduleTimer, m_pTimerQueue not inited or closed.");
		return QT_ERROR_NOT_INITIALIZED;
	}

	return m_pTimerQueue->ScheduleTimer(aTh, aArg, aInterval, aCount);
}

QtResult CQtReactorBase::CancelTimer(IQtTimerHandler *aTh)
{
	m_Est.EnsureSingleThread();
	if (!m_pTimerQueue) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::CancelTimer, m_pTimerQueue not inited or closed.");
		return QT_ERROR_NOT_INITIALIZED;
	}

	return m_pTimerQueue->CancelTimer(aTh);
}

QtResult CQtReactorBase::
ProcessHandleEvent(QT_HANDLE aFd, AQtEventHandler::MASK aMask, 
				   QtResult aReason, BOOL aIsNotify, BOOL aDropConnect)
{
	m_Est.EnsureSingleThread();
	if (aFd == QT_INVALID_HANDLE) {
		QT_ASSERTE(aMask == AQtEventHandler::EVENTQUEUE_MASK);

		// get one pending event once,
		// so that the events and signals are serial.
		// can't do this because it causes signal queue overflow.
#if 1
		DWORD dwRemainSize = 0;
		CQtEventQueueBase::EventsType listEvents;
		QtResult rv = CQtEventQueueUsingMutex::PopPendingEventsWithoutWait(
			listEvents, CQtEventQueueBase::MAX_GET_ONCE, &dwRemainSize);
		if (QT_SUCCEEDED(rv))
			rv = CQtEventQueueBase::ProcessEvents(listEvents);
		
		if (dwRemainSize)
		{
			rv = NotifyHandler(NULL, AQtEventHandler::EVENTQUEUE_MASK);
			if(QT_FAILED(rv))
				m_bNotifyFailed = TRUE;
			else if(m_bNotifyFailed)
				m_bNotifyFailed = FALSE;
		}
		return rv;
#else
		IQtEvent *pEvent = NULL;
		QtResult rv = CQtEventQueueUsingMutex::PopOnePendingEventWithoutWait(pEvent);
		if (QT_SUCCEEDED(rv))
			rv = CQtEventQueueBase::ProcessOneEvent(pEvent);
		return rv;
#endif
	}

#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
#endif // !QT_DISABLE_EVENT_REPORT

	CQtEventHandlerRepository::CElement eleFind;
	QtResult rv = m_EhRepository.Find(aFd, eleFind);
	if (QT_FAILED(rv)) {
		if (!aDropConnect) {
			QT_WARNING_TRACE_THIS("CQtReactorBase::ProcessHandleEvent, handle not registed."
				" aFd=" << aFd <<
				" aMask=" << aMask <<
				" aReason=" << aReason <<
				" rv=" << rv);
		}
		return rv;
	}

	if (QT_BIT_DISABLED(aMask, AQtEventHandler::CLOSE_MASK)) {
		AQtEventHandler::MASK maskActual = eleFind.m_Mask & aMask;
		// needn't check the registered mask if it is notify.
		if (!maskActual && !aIsNotify) {
			QT_WARNING_TRACE_THIS("CQtReactorBase::ProcessHandleEvent, mask not registed."
				" aFd=" << aFd <<
				" aMask=" << aMask <<
				" m_Mask=" << eleFind.m_Mask <<
				" aReason=" << aReason);
			return QT_OK;
		}
		
		int nOnCall = 0;
		if (aDropConnect && maskActual & AQtEventHandler::CONNECT_MASK) {
			QT_WARNING_TRACE_THIS("CQtReactorBase::ProcessHandleEvent, drop connect."
				" aFd=" << aFd <<
				" aMask=" << aMask <<
				" m_Mask=" << eleFind.m_Mask);
			nOnCall = -1;
		}
		else {
			if (maskActual & AQtEventHandler::ACCEPT_MASK
				|| maskActual & AQtEventHandler::READ_MASK)
			{
				nOnCall = eleFind.m_pEh->OnInput(aFd);
			}
			if ((nOnCall == 0 || nOnCall == -2) && 
				(maskActual & AQtEventHandler::CONNECT_MASK
				|| maskActual & AQtEventHandler::WRITE_MASK))
			{
				nOnCall = eleFind.m_pEh->OnOutput(aFd);
			}
		}

		if (nOnCall == 0) {
			rv = QT_OK;
		}
		else if (nOnCall == -2) {
			rv = QT_ERROR_WOULD_BLOCK;
		} 
		else {
			// maybe the handle is reregiested or removed when doing callbacks. 
			// so we have to refind it.
			CQtEventHandlerRepository::CElement eleFindAgain;
			rv = m_EhRepository.Find(aFd, eleFindAgain);
			if (QT_FAILED(rv) || eleFind.m_pEh != eleFindAgain.m_pEh) {
				//QT_ERROR_TRACE_THIS("CQtReactorBase::ProcessHandleEvent,"
				//	" callback shouldn't return fail after the fd is reregiested or removed!"
				//	" aFd=" << aFd << 
				//	" EHold=" << eleFind.m_pEh << 
				//	" EHnew=" << eleFindAgain.m_pEh << 
				//	" find=" << rv);
				QT_ASSERTE(FALSE);
			}
			else {
				rv = RemoveHandleWithoutFinding_i(aFd, eleFindAgain, 
					AQtEventHandler::ALL_EVENTS_MASK | AQtEventHandler::SHOULD_CALL);
			}
			rv = QT_ERROR_FAILURE;
		}
	}
	else {
//		QT_INFO_TRACE_THIS("CQtReactorBase::ProcessHandleEvent, handle is closed."
//			" aFd=" << aFd <<
//			" aMask=" << aMask <<
//			" aReason=" << aReason);

		rv = RemoveHandleWithoutFinding_i(aFd, eleFind, 
			AQtEventHandler::ALL_EVENTS_MASK | AQtEventHandler::SHOULD_CALL);
		rv = QT_ERROR_FAILURE;
	}

#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvSub = CQtTimeValue::GetTimeOfDay() - tvCur;
	if (tvSub > CQtEventQueueBase::s_tvReportInterval) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::ProcessHandleEvent, report,"
			" sec=" << tvSub.GetSec() << 
			" usec=" << tvSub.GetUsec() <<
			" aFd=" << aFd <<
			" aMask=" << aMask <<
			" maskFind=" << eleFind.m_Mask << 
			" ehFind=" << eleFind.m_pEh << 
			" aReason=" << aReason);
	}
#endif // !QT_DISABLE_EVENT_REPORT
	return rv;
}

QtResult CQtReactorBase::ProcessTimerTick()
{
#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
#endif // !QT_DISABLE_EVENT_REPORT

	m_Est.EnsureSingleThread();
	QT_ASSERTE_RETURN(m_pTimerQueue, QT_ERROR_NOT_INITIALIZED);
	if (m_pTimerQueue) 
		m_pTimerQueue->CheckExpire();

#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvSub = CQtTimeValue::GetTimeOfDay() - tvCur;
	if (tvSub > CQtEventQueueBase::s_tvReportInterval) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::ProcessTimerTick, report,"
			" sec=" << tvSub.GetSec() << 
			" usec=" << tvSub.GetUsec());
	}
#endif // !QT_DISABLE_EVENT_REPORT
	return QT_OK;
}

QtResult CQtReactorBase::
RemoveHandleWithoutFinding_i(QT_HANDLE aFd, 
						   const CQtEventHandlerRepository::CElement &aHe, 
						   AQtEventHandler::MASK aMask)
{
	AQtEventHandler::MASK maskNew = aMask & AQtEventHandler::ALL_EVENTS_MASK;
	AQtEventHandler::MASK maskEh = aHe.m_Mask;
	AQtEventHandler::MASK maskSelect = (maskEh & maskNew) ^ maskEh;
	if (maskSelect == maskEh) {
		QT_WARNING_TRACE_THIS("CQtReactorBase::RemoveHandleWithoutFinding_i, mask is equal. aMask=" << aMask);
		return QT_OK;
	}

	if (maskSelect == AQtEventHandler::NULL_MASK) {
		QtResult rv = m_EhRepository.UnBind(aFd);
		if (QT_FAILED(rv)) {
			QT_WARNING_TRACE_THIS("CQtReactorBase::RemoveHandleWithoutFinding_i, UnBind() failed!"
				" aFd=" << aFd <<
				" aMask=" << aMask <<
				" rv=" << rv);
		}
		OnHandleRemoved(aFd);
		if (aMask & AQtEventHandler::SHOULD_CALL) {
			aHe.m_pEh->OnClose(aFd, maskEh);
		}
		return QT_OK;
	}
	else {
		CQtEventHandlerRepository::CElement eleBind = aHe;
		eleBind.m_Mask = maskSelect;
		QtResult rvBind = m_EhRepository.Bind(aFd, eleBind);
		QT_ASSERTE(rvBind == QT_ERROR_FOUND);
		return rvBind;
	}
}

// this function can be invoked in the different thread.
QtResult CQtReactorBase::SendEvent(IQtEvent *aEvent)
{
	return CQtEventQueueUsingMutex::SendEvent(aEvent);
}

// this function can be invoked in the different thread.
QtResult CQtReactorBase::PostEvent(IQtEvent* aEvent, EPriority aPri)
{
	
	//it should got crash after the event already stopped, 9/2 2009
	QT_ASSERTE_RETURN(!CQtStopFlag::m_bStoppedFlag, QT_ERROR_NOT_INITIALIZED);
	if(CQtStopFlag::m_bStoppedFlag)
		QT_WARNING_TRACE_THIS("CQtReactorBase::PostEvent CQtStopFlag::m_bStoppedFlag = " << CQtStopFlag::m_bStoppedFlag);
	DWORD dwOldSize = 0;
	QtResult rv = CQtEventQueueUsingMutex::
		PostEventWithOldSize(aEvent, aPri, &dwOldSize);
	if(dwOldSize && m_bNotifyFailed)
	{
		QT_WARNING_TRACE_THIS("CQtReactorBase::PostEvent dwOldSize = " << dwOldSize);
	}
	if ((QT_SUCCEEDED(rv) && dwOldSize == 0)|| m_bNotifyFailed)
	{
		rv = NotifyHandler(NULL, AQtEventHandler::EVENTQUEUE_MASK);
		if(QT_FAILED(rv))
			m_bNotifyFailed = TRUE;
		else if(m_bNotifyFailed)
			m_bNotifyFailed = FALSE;
	}
	return rv;
}

// this function can be invoked in the different thread.
DWORD CQtReactorBase::GetPendingEventsCount()
{
	return CQtEventQueueUsingMutex::GetPendingEventsCount();
}
