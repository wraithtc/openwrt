
#include "QtBase.h"
#include "QtReactorEpoll.h"
#include "QtTimeValue.h"
#include "QtTimerQueueBase.h"

static DWORD s_dwTimerJiffies;

static void s_TimerTickFun(int )
{
//	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
//	::printf("s_TimerTickFun, sec=%ld, use=%ld jiffies=%lu. tid=%lu\n", 
//		tvCur.GetSec(), tvCur.GetUsec(), s_dwTimerJiffies, CQtThreadManager::GetThreadSelfId());

	++s_dwTimerJiffies;
}

CQtReactorEpoll::CQtReactorEpoll()
	: CQtReactorBase(SEND_REGISTER_PROPERTY)
	, m_fdEpoll(QT_INVALID_HANDLE)
	, m_pEvents(NULL)
	, m_nEventsBeginIndex(0)
	, m_nEventsEndIndex(0)
#ifdef QT_ENABLE_CALENDAR_TIMER
	, m_CalendarTimer(s_dwDefaultTimerTickInterval, 1000*60*60*2, static_cast<CQtEventQueueUsingMutex*>(this))
	, m_dwWallTimerJiffies(0)
#endif // QT_ENABLE_CALENDAR_TIMER
{
}

CQtReactorEpoll::~CQtReactorEpoll()
{
	Close();
}

QtResult CQtReactorEpoll::Open()
{
	QtResult rv;
	QT_ASSERTE_RETURN(m_fdEpoll == QT_INVALID_HANDLE, QT_ERROR_ALREADY_INITIALIZED);

	rv = CQtReactorBase::Open();
	if (QT_FAILED(rv))
		goto fail;

	QT_ASSERTE(m_EhRepository.GetMaxHandlers() > 0);

	m_fdEpoll = ::epoll_create(m_EhRepository.GetMaxHandlers());
	if (m_fdEpoll < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::Open, epoll_create() failed!"
			" max_handler=" << m_EhRepository.GetMaxHandlers() << 
			" m_fdEpoll=" << m_fdEpoll << 
			" err=" << errno);
		m_fdEpoll = QT_INVALID_HANDLE;
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	QT_ASSERTE(!m_pEvents);
	m_pEvents = new struct epoll_event[m_EhRepository.GetMaxHandlers()];
	
	rv = m_Notify.Open(this);
	if (QT_FAILED(rv))
		goto fail;

#ifdef QT_ENABLE_CALENDAR_TIMER
	if (::signal(SIGALRM, s_TimerTickFun) == SIG_ERR) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::Open, signal(SIGALARM) failed! err=" << errno);
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	struct itimerval itvInterval;
	itvInterval.it_value.tv_sec = 0;
	itvInterval.it_value.tv_usec = 100;
	itvInterval.it_interval.tv_sec = 0;
	itvInterval.it_interval.tv_usec = s_dwDefaultTimerTickInterval * 1000;
	if (::setitimer(ITIMER_REAL, &itvInterval, NULL) == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::Open, setitimer() failed! err=" << errno);
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	m_CalendarTimer.m_Est.Reset2CurrentThreadId();
#endif // QT_ENABLE_CALENDAR_TIMER

	CQtStopFlag::SetStartFlag();
	QT_INFO_TRACE_THIS("CQtReactorEpoll::Open, successful,"
		" max_handler=" << m_EhRepository.GetMaxHandlers() << 
		" m_fdEpoll=" << m_fdEpoll);
	return QT_OK;

fail:
	Close();
	QT_ASSERTE(QT_FAILED(rv));
	return rv;
}

QtResult CQtReactorEpoll::NotifyHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	return m_Notify.Notify(aEh, aMask);
}

QtResult CQtReactorEpoll::RunEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorEpoll::RunEventLoop");
	m_Est.EnsureSingleThread();

	while (!CQtStopFlag::m_bStoppedFlag) {
#ifdef QT_ENABLE_CALENDAR_TIMER
		// <s_dwTimerJiffies> alaways be greater than <m_dwWallTimerJiffies> even if it equals 0, 
		// becausae <m_dwWallTimerJiffies> is increased following by <s_dwTimerJiffies>.
		DWORD dwTimerJiffiesTmp = s_dwTimerJiffies;
		DWORD dwTicks = dwTimerJiffiesTmp - m_dwWallTimerJiffies;

//		CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
//		::printf("CQtReactorEpoll::RunEventLoop, sec=%ld, use=%ld dwTimerJiffiesTmp=%lu"
//			" m_dwWallTimerJiffies=%lu dwTicks=%lu.\n", 
//			tvCur.GetSec(), tvCur.GetUsec(), dwTimerJiffiesTmp, m_dwWallTimerJiffies, dwTicks);

		QT_ASSERTE(dwTicks < 0xFFFF0000);
		if (dwTicks > 33) {
			QT_ERROR_TRACE_THIS("CQtReactorEpoll::RunEventLoop, time too long."
				" dwTimerJiffiesTmp=" << dwTimerJiffiesTmp << 
				" m_dwWallTimerJiffies=" << m_dwWallTimerJiffies << 
				" dwTicks=" << dwTicks);
		}
		
		m_dwWallTimerJiffies += dwTicks;

		while (dwTicks-- > 0)
			m_CalendarTimer.TimerTick();

		int nTimeout = (int)s_dwDefaultTimerTickInterval;
#else // !QT_ENABLE_CALENDAR_TIMER
		CQtTimeValue tvTimeout(CQtTimeValue::s_tvMax);
		if (m_pTimerQueue) {
			// process timer prior to wait event.
			m_pTimerQueue->CheckExpire(&tvTimeout);
		}
		
		int nTimeout = -1;
		if (tvTimeout != CQtTimeValue::s_tvMax)
			nTimeout = tvTimeout.GetTotalInMsec();
#endif // QT_ENABLE_CALENDAR_TIMER
		//QT_INFO_TRACE_THIS("CQtReactorEpoll::RunEventLoop, epoll_wait again ");
		int nRetFds = ::epoll_wait(m_fdEpoll, m_pEvents, m_EhRepository.GetMaxHandlers(), nTimeout);
		if (nRetFds < 0) {
			if (errno == EINTR)
				continue;
			
			QT_ERROR_TRACE_THIS("CQtReactorEpoll::RunEventLoop, epoll_wait() failed!"
				" max_handler=" << m_EhRepository.GetMaxHandlers() << 
				" m_fdEpoll=" << m_fdEpoll << 
				" nTimeout=" << nTimeout << 
				" err=" << errno);
			return QT_ERROR_FAILURE; 
		}

		m_nEventsEndIndex = nRetFds;
		struct epoll_event *pEvent = m_pEvents;
		for (m_nEventsBeginIndex = 0; m_nEventsBeginIndex < m_nEventsEndIndex; m_nEventsBeginIndex++, pEvent++) {
	//		QT_INFO_TRACE_THIS("CQtReactorEpoll::RunEventLoop,"
	//			" m_nEventsBeginIndex=" << m_nEventsBeginIndex << 
	//			" m_nEventsEndIndex=" << m_nEventsEndIndex << 
	//			" fd=" << pEvent->data.fd << 
	//			" events=" << pEvent->events);

			// TODO: 
			// 1. test UDP, packets merge?
			// 2. test send data before wait.
			// 3. test SSL, read return value less than parameter
			// 4. PUSH & FIN before epoll_wait().
			// 5. PUSH & FIN before epoll_ctl().

			QtResult rvError = QT_OK;
			int fdSig = pEvent->data.fd;
			// fdSing may be modified to QT_INVALID_HANDLE due to OnHandleRemoved() function.
			if (fdSig == QT_INVALID_HANDLE)
				continue;
				
			AQtEventHandler::MASK maskSig = AQtEventHandler::NULL_MASK;
			long lSigEvent = pEvent->events;
			if (lSigEvent & (EPOLLERR | EPOLLHUP)) {
				QT_WARNING_TRACE_THIS("CQtReactorEpoll::RunEventLoop,"
					" handle is closed."
					" lSigEvent=" << lSigEvent << 
					" fd=" << fdSig <<
					" band=" << lSigEvent << 
					" m_nEventsBeginIndex=" << m_nEventsBeginIndex);
				rvError = QT_ERROR_NETWORK_SOCKET_CLOSE;
				QT_SET_BITS(maskSig, AQtEventHandler::CLOSE_MASK);
			}
			else {
				if (lSigEvent & EPOLLIN)
					maskSig |= AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK;
				if (lSigEvent & EPOLLOUT)
					maskSig |= AQtEventHandler::WRITE_MASK | AQtEventHandler::CONNECT_MASK;
			}
			
			ProcessHandleEvent(fdSig, maskSig, rvError, FALSE);
		}
		
		m_nEventsBeginIndex = 0;
		m_nEventsEndIndex = 0;
	}
	
	return QT_OK;
}

// this function can be invoked in the different thread.
QtResult CQtReactorEpoll::StopEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorEpoll::StopEventLoop");

	CQtEventQueueUsingMutex::Stop();
	CQtStopFlag::m_bStoppedFlag = TRUE;
	return QT_OK;
}

QtResult CQtReactorEpoll::Close()
{
#ifdef QT_ENABLE_CALENDAR_TIMER
	if (::signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::Close, signal(SIGALARM) failed! err=" << errno);
	}
	struct itimerval itvInterval;
	itvInterval.it_value.tv_sec = 0;
	itvInterval.it_value.tv_usec = 0;
	itvInterval.it_interval.tv_sec = 0;
	itvInterval.it_interval.tv_usec = 0;
	if (::setitimer(ITIMER_REAL, &itvInterval, NULL) == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::Close, setitimer() failed! err=" << errno);
	}
	m_dwWallTimerJiffies = 0;
#endif // QT_ENABLE_CALENDAR_TIMER

	if (m_fdEpoll != QT_INVALID_HANDLE) {
		::close(m_fdEpoll);
		m_fdEpoll = QT_INVALID_HANDLE;
	}
	if (m_pEvents) {
		delete []m_pEvents;
		m_pEvents = NULL;
	}
	
	m_Notify.Close();
	return CQtReactorBase::Close();
}

QtResult CQtReactorEpoll::
OnHandleRegister(QT_HANDLE aFd, AQtEventHandler::MASK aMask, AQtEventHandler *aEh)
{
	// Need NOT do CheckPollIn() because epoll_ctl() will do it interval.
	return DoEpollCtl_i(aFd, aMask, EPOLL_CTL_ADD);;
}

void CQtReactorEpoll::OnHandleRemoved(QT_HANDLE aFd)
{
//	QT_INFO_TRACE_THIS("CQtReactorEpoll::OnHandleRemoved,"
//		" aFd=" << aFd << 
//		" m_nEventsBeginIndex=" << m_nEventsBeginIndex << 
//		" m_nEventsEndIndex=" << m_nEventsEndIndex);

	if (::epoll_ctl(m_fdEpoll, EPOLL_CTL_DEL, aFd, NULL) < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::OnHandleRemoved, epoll_ctl() failed!"
			" m_fdEpoll=" << m_fdEpoll << 
			" aFd=" << aFd << 
			" err=" << errno);
	}
	
	// budingc, 2/6/2006, remove fd from m_pEvents cache.
	if (m_nEventsEndIndex == 0)
		return;

	int i = m_nEventsBeginIndex + 1;
	struct epoll_event *pEvent = m_pEvents + i;
	for ( ; i < m_nEventsEndIndex; i++, pEvent++) {
		if (pEvent->data.fd == aFd) {
			QT_WARNING_TRACE_THIS("CQtReactorEpoll::OnHandleRemoved,"
				" find same fd=" << aFd << 
				" m_nEventsBeginIndex=" << m_nEventsBeginIndex << 
				" m_nEventsEndIndex=" << m_nEventsEndIndex << 
				" i=" << i);
			pEvent->data.fd = QT_INVALID_HANDLE;
			return;
		}
	}
}

QtResult CQtReactorEpoll::
RegisterHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
//	QT_INFO_TRACE_THIS("CQtReactorEpoll::RegisterHandler"
//		" aEh=" << aEh << " aMask=" << aMask << 
//		" fd=" << (aEh ? aEh->GetHandle() : QT_INVALID_HANDLE) << 
//		" m_nEventsBeginIndex=" << m_nEventsBeginIndex << 
//		" m_nEventsEndIndex=" << m_nEventsEndIndex);

	QtResult rv = CQtReactorBase::RegisterHandler(aEh, aMask);
	if (rv == QT_ERROR_FOUND) {
		rv = DoEpollCtl_i(aEh->GetHandle(), aMask, EPOLL_CTL_MOD);
		if (QT_SUCCEEDED(rv))
			rv = QT_ERROR_FOUND;
	}
	return rv;
}

QtResult CQtReactorEpoll::
RemoveHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
//	QT_INFO_TRACE_THIS("CQtReactorEpoll::RemoveHandler"
//		" aEh=" << aEh << " aMask=" << aMask << 
//		" fd=" << (aEh ? aEh->GetHandle() : QT_INVALID_HANDLE) << 
//		" m_nEventsBeginIndex=" << m_nEventsBeginIndex << 
//		" m_nEventsEndIndex=" << m_nEventsEndIndex);

	QtResult rv = CQtReactorBase::RemoveHandler(aEh, aMask);
	if (rv == QT_ERROR_FOUND) {
		rv = DoEpollCtl_i(aEh->GetHandle(), aMask, EPOLL_CTL_MOD);
		if (QT_SUCCEEDED(rv))
			rv = QT_ERROR_FOUND;
	}
	return rv;
}

QtResult CQtReactorEpoll::DoEpollCtl_i(QT_HANDLE aFd, AQtEventHandler::MASK aMask, int aOperation)
{
	struct epoll_event epEvent;
	::memset(&epEvent, 0, sizeof(epEvent));
	epEvent.events = EPOLLERR | EPOLLHUP;
	//Modify 2006.4.4 UDP can not use edge model, otherwise some data will miss
	if(!(aMask & AQtEventHandler::ACCEPT_MASK) && !(aMask & AQtEventHandler::UDP_LINK_MASK)) //Add 17/02 2006 accept with level model
	{
		//epEvent.events |= EPOLLET;  //修改TCP，收到请求在很短的时间内收到断开连接FIN，使得FIN无法获取到时间的问题
	}
	
	epEvent.data.fd = aFd;

	if (aMask & AQtEventHandler::CONNECT_MASK)
		epEvent.events |= EPOLLIN | EPOLLOUT;
	if (aMask & AQtEventHandler::READ_MASK || aMask & AQtEventHandler::ACCEPT_MASK)
		epEvent.events |= EPOLLIN;
	if (aMask & AQtEventHandler::WRITE_MASK)
		epEvent.events |= EPOLLOUT;

	if (::epoll_ctl(m_fdEpoll, aOperation, aFd, &epEvent) < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorEpoll::DoEpollCtl_i, epoll_ctl() failed!"
			" m_fdEpoll=" << m_fdEpoll << 
			" aFd=" << aFd << 
			" aOperation=" << aOperation << 
			" err=" << errno);
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
}

#ifdef QT_ENABLE_CALENDAR_TIMER
QtResult CQtReactorEpoll::
ScheduleTimer(IQtTimerHandler *aTh, LPVOID aArg, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	return m_CalendarTimer.ScheduleTimer(aTh, aArg, aInterval, aCount);
}

QtResult CQtReactorEpoll::CancelTimer(IQtTimerHandler *aTh)
{
	return m_CalendarTimer.CancelTimer(aTh);
}
#endif // QT_ENABLE_CALENDAR_TIMER

