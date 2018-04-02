

#include "QtBase.h"
#include "QtReactorRealTimeSignal.h"
#include "QtTimeValue.h"
#include "QtTimerQueueBase.h"

//_syscall1(int, _sysctl, struct __sysctl_args *, args);

//#define QT_ENABLE_PRINT_TIMER_TICK

CQtReactorRealTimeSignal::CQtReactorRealTimeSignal()
	: m_SigNum(SIGRTMIN)
	, m_nPid(0)
	, m_bSignalOverFlowing(FALSE)
#ifdef QT_ENABLE_CALENDAR_TIMER
	, m_CalendarTimer(s_dwDefaultTimerTickInterval, 1000*60*60*2, static_cast<CQtEventQueueUsingMutex*>(this))
#endif // QT_ENABLE_CALENDAR_TIMER
{
}

CQtReactorRealTimeSignal::~CQtReactorRealTimeSignal()
{
	Close();
}

static CQtReactorRealTimeSignal *s_ReactorRealTimeSignal;
QT_THREAD_ID s_tidNetwork;

void CQtReactorRealTimeSignal::TimerTickFun(int )
{
#ifdef QT_ENABLE_PRINT_TIMER_TICK
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
	::printf("CQtReactorRealTimeSignal::TimerTickFun, sec=%ld, use=%ld.\n",
		tvCur.GetSec(), tvCur.GetUsec());
#endif // QT_ENABLE_PRINT_TIMER_TICK

//	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(s_tidNetwork));
	
	if (s_ReactorRealTimeSignal)
		s_ReactorRealTimeSignal->Sigqueue_i(SV_TIMER, FALSE);
}

QtResult CQtReactorRealTimeSignal::Open()
{
	QtResult rv = QT_ERROR_UNEXPECTED;
	int nRet;
	
	nRet = ::sigemptyset(&m_Sigset);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, sigemptyset() failed! err=" << errno);
		return rv;
	}
	nRet = ::sigaddset(&m_Sigset, m_SigNum);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, sigaddset(m_SigNum) failed! err=" << errno);
		return rv;
	}
	// Put SIGIO in the same set, since we need to listen for that, too.
	nRet = ::sigaddset(&m_Sigset, SIGIO);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, sigaddset(SIGIO) failed! err=" << errno);
		return rv;
	}
	// Finally, block delivery of those signals.
	nRet = ::sigprocmask(SIG_BLOCK, &m_Sigset, NULL);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, sigprocmask() failed! err=" << errno);
		return rv;
	}

	// don't care whether SetProcRtsigMax_i() failed or not.
	int nMaxSig = SetProcRtsigMax_i(8192*10);

	rv = CQtReactorBase::Open();
	if (QT_FAILED(rv))
		goto fail;

	// getpid prior to m_Notify.Open() because m_Notify will do register handler.
	m_nPid = getpid();

	rv = m_Notify.Open(this);
	if (QT_FAILED(rv))
		goto fail;

#ifdef QT_ENABLE_CALENDAR_TIMER
	QT_ASSERTE(!s_ReactorRealTimeSignal);
	if (::signal(SIGALRM, TimerTickFun) == SIG_ERR) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, signal(SIGALARM) failed! err=" << errno);
		rv = QT_ERROR_FAILURE;
		goto fail;
	}
	s_ReactorRealTimeSignal = this;
	s_tidNetwork = CQtThreadManager::GetThreadSelfId();

	struct itimerval itvInterval;
	itvInterval.it_value.tv_sec = 0;
	itvInterval.it_value.tv_usec = 100;
	itvInterval.it_interval.tv_sec = 0;
	itvInterval.it_interval.tv_usec = s_dwDefaultTimerTickInterval * 1000;
	nRet = ::setitimer(ITIMER_REAL, &itvInterval, NULL);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, setitimer() failed! err=" << errno);
//		::printf("CQtReactorRealTimeSignal::Open, setitimer() failed! err=%d.\n", errno);
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	m_CalendarTimer.m_Est.Reset2CurrentThreadId();
#endif // QT_ENABLE_CALENDAR_TIMER

	CQtStopFlag::SetStartFlag();
	QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::Open, successful,"
		" MaxSig=" << nMaxSig);
	return rv;

fail:
	Close();
	QT_ASSERTE(QT_FAILED(rv));
	QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Open, failed!"
		" rv=" << rv);
	return rv;
}

QtResult CQtReactorRealTimeSignal::
NotifyHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
//	QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::NotifyHandler");

	if (!aEh) {
		QT_ASSERTE(aMask == AQtEventHandler::EVENTQUEUE_MASK);
		return Sigqueue_i(SV_EVENT);
	}
	else {
		return m_Notify.Notify(aEh, aMask);
	}
}

QtResult CQtReactorRealTimeSignal::ProcessSignalOverFlow()
{
	QtResult rv;
	QT_ERROR_TRACE_THIS("CReactorRealTimeSignal::ProcessSignalOverFlow,"
		" err=" << errno << 
		" m_bSignalOverFlowing=" << m_bSignalOverFlowing);
	
	::printf("CReactorRealTimeSignal::ProcessSignalOverFlow." 
		" err=%d m_bSignalOverFlowing=%d\n",
		errno, (int)m_bSignalOverFlowing);
				
	if (m_bSignalOverFlowing)
		return QT_OK;
	m_bSignalOverFlowing = TRUE;

	int nMaxHandler = m_EhRepository.GetMaxHandlers();
	for (int i = 0; i < nMaxHandler; i++) {
		rv = ProcessHandleEvent(
			i, 
			AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK | AQtEventHandler::WRITE_MASK,
			QT_OK,
			FALSE,
			TRUE);
	}

	rv = ProcessHandleEvent(QT_INVALID_HANDLE, 
					   AQtEventHandler::EVENTQUEUE_MASK, 
					   QT_OK, 
					   FALSE);
	return rv;
}

QtResult CQtReactorRealTimeSignal::ProcessOneSignalFd(int aFd, long int aBand)
{
	QtResult rvError = QT_OK;
	AQtEventHandler::MASK maskSig = AQtEventHandler::NULL_MASK;
	if (aBand & (POLLERR|POLLHUP|POLLNVAL)) {
		QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::ProcessOneSignalFd,"
			" handle is closed."
			" aFd=" << aFd <<
			" band=" << aBand);
		rvError = QT_ERROR_NETWORK_SOCKET_CLOSE;
		QT_SET_BITS(maskSig, AQtEventHandler::CLOSE_MASK);
	}
	else {
		if (aBand & POLLIN)
			maskSig |= AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK;
		if (aBand & POLLOUT)
			maskSig |= AQtEventHandler::WRITE_MASK | AQtEventHandler::CONNECT_MASK;
	}
	
	QtResult rv = ProcessHandleEvent(aFd, maskSig, rvError, FALSE);
	return rv;
}

QtResult CQtReactorRealTimeSignal::ProcessOneSignalQueue(int aVal)
{
	QtResult rv = QT_OK;
	if (aVal == SV_EVENT) {
//		QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::ProcessOneSignalQueue, SI_QUEUE SV_EVENT.");
		rv = ProcessHandleEvent(QT_INVALID_HANDLE, AQtEventHandler::EVENTQUEUE_MASK, QT_OK, FALSE);
	}
	else if (aVal == SV_STOP) {
		QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::ProcessOneSignalQueue, reactor is stopped.");
		CQtStopFlag::SetStopFlag();
	}
#ifdef QT_ENABLE_CALENDAR_TIMER
	else if (aVal == SV_TIMER) {
#ifdef QT_ENABLE_PRINT_TIMER_TICK
		CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
		::printf("CQtReactorRealTimeSignal::ProcessOneSignalQueue, SV_TIMER, sec=%ld, use=%ld.\n",
			tvCur.GetSec(), tvCur.GetUsec());
#endif // QT_ENABLE_PRINT_TIMER_TICK
		
		m_CalendarTimer.TimerTick();
	}
#endif // QT_ENABLE_CALENDAR_TIMER
	else {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::ProcessOneSignalQueue,"
			" unknow value=" << aVal);
		QT_ASSERTE(FALSE);
	}
	return rv;
}

QtResult CQtReactorRealTimeSignal::RunEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::RunEventLoop");
	m_Est.EnsureSingleThread();

//	while (!CQtStopFlag::IsFlagStopped()) {
	while (!CQtStopFlag::m_bStoppedFlag) {
		struct timespec *pTs = NULL;
#ifndef QT_ENABLE_CALENDAR_TIMER
		// improve the performance.
		CQtTimeValue tvTimeout(CQtTimeValue::s_tvMax);
		if (m_pTimerQueue) {
			// process timer prior to wait event.
			m_pTimerQueue->CheckExpire(&tvTimeout);
		}
		struct timespec tsBuf;
		if (tvTimeout != CQtTimeValue::s_tvMax) {
			tsBuf.tv_sec = tvTimeout.GetSec();
			tsBuf.tv_nsec = tvTimeout.GetUsec() * 1000;
			pTs = &tsBuf;
		}
#endif // !QT_ENABLE_CALENDAR_TIMER
		// make ProcessSignal_i() inline.
		siginfo_t siginfo;
		int sigRet;
		if (pTs) {
			sigRet = ::sigtimedwait(&m_Sigset, &siginfo, pTs);
			if (sigRet == -1 && errno == EAGAIN)
				continue;
		}
		else {
			sigRet = ::sigwaitinfo(&m_Sigset, &siginfo);
		}

		if (sigRet == -1 || sigRet == SIGIO) {
			if (sigRet == -1 && errno == EINTR)
				continue;
			else {
				QT_ERROR_TRACE_THIS("CReactorRealTimeSignal::RunEventLoop,"
					" sigtimedwait() failed!"
					" sigRet=" << sigRet << 
					" err=" << errno << 
					" m_bSignalOverFlowing=" << m_bSignalOverFlowing);

				::printf("CReactorRealTimeSignal::RunEventLoop." 
					" sigtimedwait() failed! sigRet=%d err=%d m_bSignalOverFlowing=%d\n",
					sigRet, errno, (int)m_bSignalOverFlowing);
				
				if (m_bSignalOverFlowing)
					continue;
				m_bSignalOverFlowing = TRUE;

				int nMaxHandler = m_EhRepository.GetMaxHandlers();
				for (int i = 0; i < nMaxHandler; i++) {
					ProcessHandleEvent(
						i, 
						AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK | AQtEventHandler::WRITE_MASK,
						QT_OK,
						FALSE,
						TRUE);
				}
				ProcessHandleEvent(
					QT_INVALID_HANDLE, 
					AQtEventHandler::EVENTQUEUE_MASK, 
					QT_OK, 
					FALSE);
				continue;
			}
		}
		
		QT_ASSERTE(sigRet == m_SigNum);
		m_bSignalOverFlowing = FALSE;
		
		if (siginfo.si_code == SI_QUEUE) {
			if (siginfo.si_value.sival_int == SV_EVENT) {
//				QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::RunEventLoop, SI_QUEUE SV_EVENT.");
				ProcessHandleEvent(QT_INVALID_HANDLE, 
					AQtEventHandler::EVENTQUEUE_MASK, QT_OK, FALSE);
			}
			else if (siginfo.si_value.sival_int == SV_STOP) {
//				QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::RunEventLoop, reactor is stopped.");
				CQtStopFlag::SetStopFlag();
			}
#ifdef QT_ENABLE_CALENDAR_TIMER
			else if (siginfo.si_value.sival_int == SV_TIMER) {
#ifdef QT_ENABLE_PRINT_TIMER_TICK
				CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
				::printf("CQtReactorRealTimeSignal::RunEventLoop, SV_TIMER, sec=%ld, use=%ld.\n",
					tvCur.GetSec(), tvCur.GetUsec());
#endif // QT_ENABLE_PRINT_TIMER_TICK

				m_CalendarTimer.TimerTick();
			}
#endif // QT_ENABLE_CALENDAR_TIMER
			else {
				QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::RunEventLoop,"
					" unknow value=" << siginfo.si_value.sival_int);
				QT_ASSERTE(FALSE);
			}
			continue;
		}

		QtResult rvError = QT_OK;
		int fdSig = siginfo.si_fd;
		AQtEventHandler::MASK maskSig = AQtEventHandler::NULL_MASK;
		long lSigEvent = siginfo.si_band;
		if (lSigEvent & (POLLERR|POLLHUP|POLLNVAL)) {
			QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::RunEventLoop,"
				" handle is closed."
				" lSigEvent=" << lSigEvent << 
				" fd=" << fdSig <<
				" band=" << lSigEvent);
			rvError = QT_ERROR_NETWORK_SOCKET_CLOSE;
			QT_SET_BITS(maskSig, AQtEventHandler::CLOSE_MASK);
		}
		else {
			if (lSigEvent & POLLIN)
				maskSig |= AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK;
			if (lSigEvent & POLLOUT)
				maskSig |= AQtEventHandler::WRITE_MASK | AQtEventHandler::CONNECT_MASK;
		}
		
		ProcessHandleEvent(fdSig, maskSig, rvError, FALSE);
	}
	return QT_OK;
}

// this function can be invoked in the different thread.
QtResult CQtReactorRealTimeSignal::StopEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorRealTimeSignal::StopEventLoop");

	// process all events before set quit the reactor.
//	NotifyHandler(&m_Notify, AQtEventHandler::CLOSE_MASK);

	QtResult rv = Sigqueue_i(SV_STOP);
	if (QT_FAILED(rv))
		CQtStopFlag::m_bStoppedFlag = TRUE;
	CQtEventQueueUsingMutex::Stop();
	return QT_OK;
}

QtResult CQtReactorRealTimeSignal::Close()
{
#ifdef QT_ENABLE_CALENDAR_TIMER
//	QT_ASSERTE(s_ReactorRealTimeSignal);
	if (::signal(SIGALRM, SIG_IGN) == SIG_ERR) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Close, signal(SIGALARM) failed! err=" << errno);
	}
	s_ReactorRealTimeSignal = NULL;

	struct itimerval itvInterval;
	itvInterval.it_value.tv_sec = 0;
	itvInterval.it_value.tv_usec = 0;
	itvInterval.it_interval.tv_sec = 0;
	itvInterval.it_interval.tv_usec = 50 * 1000; // 50ms
	int nRet = ::setitimer(ITIMER_REAL, &itvInterval, NULL);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Close, setitimer() failed! err=" << errno);
//		::printf("CQtReactorRealTimeSignal::Close, setitimer() failed! err=%d.\n", errno);
	}
#endif // QT_ENABLE_CALENDAR_TIMER

	m_Notify.Close();
	return CQtReactorBase::Close();
}

QtResult CQtReactorRealTimeSignal::
OnHandleRegister(QT_HANDLE aFd, AQtEventHandler::MASK aMask, AQtEventHandler *aEh)
{
	QtResult rv = QT_ERROR_UNEXPECTED;
	int nflags, nFcntl;

	// Set this fd to emit signals.
	nFcntl = ::fcntl(aFd, F_GETFL, &nflags);
	if (nFcntl < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRegister, fcntl(F_GETFL) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
		return rv;
	}
	
	// Set socket flags to non-blocking and asynchronous 
	nflags |= O_RDWR | O_NONBLOCK | O_ASYNC;
	nFcntl = ::fcntl(aFd, F_SETFL, nflags);
	if (nFcntl < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRegister, fcntl(F_SETFL) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
		return rv;
	}
	
	// Set signal number >= SIGRTMIN to send a RealTime signal 
	nFcntl = ::fcntl(aFd, F_SETSIG, m_SigNum);
	if (nFcntl < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRegister, fcntl(F_SETSIG) failed!"
			" nFcntl=" << nFcntl <<
			" m_SigNum=" << m_SigNum <<
			" err=" << errno);
		return rv;
	}
	
	// Set process id to send signal to 
	nFcntl = ::fcntl(aFd, F_SETOWN, m_nPid);
	if (nFcntl < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRegister, fcntl(F_SETOWN) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
		return rv;
	}
	
#ifdef F_SETAUXFL
	// Allow only one signal per socket fd 
	nFcntl = ::fcntl(aFd, F_SETAUXFL, O_ONESIGFD);
	if (nFcntl < 0) {
		QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRegister, fcntl(F_SETAUXFL) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
		return rv;
	}
#endif // F_SETAUXFL

	// connector should register this before call ::connect, 
	// so check if there is data to read except connect mask
	if (QT_BIT_DISABLED(aMask, AQtEventHandler::CONNECT_MASK))
		return CheckPollIn_i(aFd, aEh);
	else
		return QT_OK;
}

void CQtReactorRealTimeSignal::OnHandleRemoved(QT_HANDLE aFd)
{
	int nflags, nFcntl;
	nFcntl = ::fcntl(aFd, F_GETFL, &nflags);
	if (nFcntl < 0) {
		QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRemoved, fcntl(F_GETFL) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
	}
	
	nflags &= ~O_ASYNC;
	nFcntl = ::fcntl(aFd, F_SETFL, nflags);
	if (nFcntl < 0) {
		QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::OnHandleRemoved, fcntl(F_SETFL) failed!"
			" nFcntl=" << nFcntl <<
			" err=" << errno);
	}
}

int CQtReactorRealTimeSignal::SetProcRtsigMax_i(int aMaxNum)
{
	int nSigMax = 0;
	size_t nNumLen = sizeof(nSigMax);
	int pName[] = { CTL_KERN, KERN_RTSIGMAX };
	
	struct __sysctl_args args;
	args.name = pName;
	args.nlen = sizeof(pName)/sizeof(pName[0]);
	args.oldval = &nSigMax;
	args.oldlenp = &nNumLen;
	args.newval = NULL;
	args.newlen = 0;
	
//	if (_sysctl(&args) == -1) {
	if (syscall(__NR__sysctl, &args) == -1) {
		QT_ERROR_TRACE_THIS("CReactorRealTimeSignal::SetProcRtsigMax_i, _sysctl(get) failed!"
			" err=" << errno);
		return -1;
	}
	
	if (aMaxNum > nSigMax) {
		int nNewSigMax = aMaxNum;
		args.oldval = NULL;
		args.oldlenp = 0;
		args.newval = &nNewSigMax;
		args.newlen = sizeof(nNewSigMax);
	//	if (_sysctl(&args) == -1) {
		if (syscall(__NR__sysctl, &args) == -1) {
			if (EPERM == errno) {
				QT_WARNING_TRACE_THIS("CReactorRealTimeSignal::SetProcRtsigMax_i, _sysctl(set) failed. "
					"you should use superuser to _sysctl(rtsig-max)!");
				return nSigMax;
			}
			else {
				QT_ERROR_TRACE_THIS("CReactorRealTimeSignal::SetProcRtsigMax_i, _sysctl(set) failed!"
					" err=" << errno);
				return -1;
			}
		}
	}
	else
		aMaxNum = nSigMax;
	return aMaxNum;
}

QtResult CQtReactorRealTimeSignal::
CheckPollIn_i(QT_HANDLE aFd, AQtEventHandler *aEh)
{
	struct pollfd pfRead[1];
	pfRead[0].fd = aFd;
	pfRead[0].events = POLLIN | POLLERR|POLLHUP|POLLNVAL;
	pfRead[0].revents = 0;
	int nReady = ::poll(pfRead, 1, 0);
	if (nReady < 0) {
		QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::CheckPollIn_i, poll() failed!"
			" aFd=" << aFd <<
			" err=" << errno);
		return QT_ERROR_FAILURE;
	}
	else if (nReady > 0) {
		if (pfRead[0].revents & POLLIN)
		{
//			QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignal::CheckPollIn_i, poll(POLLIN)."
//				" aFd=" << aFd <<
//				" revents=" << pfRead[0].revents);
			return NotifyHandler(aEh, AQtEventHandler::READ_MASK);
		}
		else if (pfRead[0].revents & (POLLERR|POLLHUP|POLLNVAL)) {
			QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::CheckPollIn_i, poll(POLLERR)."
				" aFd=" << aFd <<
				" revents=" << pfRead[0].revents << 
				" errno = " << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else {
			QT_WARNING_TRACE_THIS("CReactorRealTimeSignal::CheckPollIn, poll(unknow)."
				" aFd=" << aFd <<
				" revents=" << pfRead[0].revents);
			return QT_OK;
		}
	}
	else
		return QT_OK;
}

#ifdef QT_ENABLE_CALENDAR_TIMER
QtResult CQtReactorRealTimeSignal::
ScheduleTimer(IQtTimerHandler *aTh, LPVOID aArg, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	return m_CalendarTimer.ScheduleTimer(aTh, aArg, aInterval, aCount);
}

QtResult CQtReactorRealTimeSignal::CancelTimer(IQtTimerHandler *aTh)
{
	return m_CalendarTimer.CancelTimer(aTh);
}
#endif // QT_ENABLE_CALENDAR_TIMER



CQtReactorRealTimeSignalBuffer::CQtReactorRealTimeSignalBuffer()
	: m_pSigalNodesIndex(NULL)
	, m_nSigalNodeIndexNumber(0)
{
}

CQtReactorRealTimeSignalBuffer::~CQtReactorRealTimeSignalBuffer()
{
	Close();
}

QtResult CQtReactorRealTimeSignalBuffer::Open()
{
	QtResult rv;
	rv = CQtReactorRealTimeSignal::Open();
	if (QT_FAILED(rv))
		return rv;

	m_nSigalNodeIndexNumber = m_EhRepository.GetMaxHandlers();
	QT_ASSERTE(m_nSigalNodeIndexNumber > 0);
	m_pSigalNodesIndex = new CSigalNode *[m_nSigalNodeIndexNumber];
	::memset(m_pSigalNodesIndex, 0, sizeof(CSigalNode *) * m_nSigalNodeIndexNumber);

	return QT_OK;
}

QtResult CQtReactorRealTimeSignalBuffer::Close()
{
	m_Est.EnsureSingleThread();
	m_SigalNodes.clear();
	if (m_pSigalNodesIndex) {
		delete [] m_pSigalNodesIndex;
		m_pSigalNodesIndex = NULL;
	}
	m_nSigalNodeIndexNumber = 0;
	
	return CQtReactorRealTimeSignal::Close();
}

void CQtReactorRealTimeSignalBuffer::
InsertOneSigalNode_i(siginfo_t &aSigInfo)
{
	if (aSigInfo.si_code != SI_QUEUE) {
		CSigalNode *pNode = m_pSigalNodesIndex[aSigInfo.si_fd];
		if (pNode) {
			QT_ASSERTE(aSigInfo.si_fd == pNode->m_CodeIo.m_si_fd);
			pNode->m_CodeIo.m_si_band |= aSigInfo.si_band;
			if (aSigInfo.si_band | POLLIN)
				pNode->m_CodeIo.m_dwReadCount++;
			return;
		}
	}

	CSigalNode sigNode;
	sigNode.m_si_code = aSigInfo.si_code;
	if (aSigInfo.si_code == SI_QUEUE)
		sigNode.m_sival_int = aSigInfo.si_value.sival_int;
	else {
		sigNode.m_CodeIo.m_si_fd = aSigInfo.si_fd;
		sigNode.m_CodeIo.m_si_band = aSigInfo.si_band;
		if (aSigInfo.si_band | POLLIN)
			sigNode.m_CodeIo.m_dwReadCount++;
	}
	SigalNodesType::iterator iter = m_SigalNodes.insert(m_SigalNodes.end(), sigNode);
	if (aSigInfo.si_code != SI_QUEUE)
		m_pSigalNodesIndex[aSigInfo.si_fd] = &(*iter);
}

QtResult CQtReactorRealTimeSignalBuffer::RunEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorRealTimeSignalBuffer::RunEventLoop");
	m_Est.EnsureSingleThread();

	while (!CQtStopFlag::m_bStoppedFlag) {
		int sigRet = 0;
		siginfo_t sigInfo;
#ifdef QT_ENABLE_CALENDAR_TIMER
		sigRet = ::sigwaitinfo(&m_Sigset, &sigInfo);
#else
		CQtTimeValue tvTimeout(CQtTimeValue::s_tvMax);
		if (m_pTimerQueue) 
			m_pTimerQueue->CheckExpire(&tvTimeout);
		if (tvTimeout != CQtTimeValue::s_tvMax) {
			struct timespec tsBuf;
			tsBuf.tv_sec = tvTimeout.GetSec();
			tsBuf.tv_nsec = tvTimeout.GetUsec() * 1000;
			sigRet = ::sigtimedwait(&m_Sigset, &sigInfo, &tsBuf);
		}
		else
			sigRet = ::sigwaitinfo(&m_Sigset, &sigInfo);
#endif // QT_ENABLE_CALENDAR_TIMER
		
		if (sigRet == -1) {
			QT_ASSERTE(errno == EAGAIN || errno == EINTR);
			continue;
		}
		else if (sigRet == SIGIO) {
			ProcessSignalOverFlow();
			continue;
		}

		QT_ASSERTE(sigRet == m_SigNum);
		m_bSignalOverFlowing = FALSE;

		BOOL bFirstNotInserted = TRUE;
		int nGetCount = 0;
		CQtTimeValue tvCurOld = CQtTimeValue::GetTimeOfDay();
		for ( ; ; ) {
			siginfo_t sigInfo2;
			struct timespec tsBuf;
			tsBuf.tv_sec = 0;
			tsBuf.tv_nsec = 0;
			sigRet = ::sigtimedwait(&m_Sigset, &sigInfo2, &tsBuf);
			if (sigRet == m_SigNum) {
				if (bFirstNotInserted) {
					InsertOneSigalNode_i(sigInfo);
					bFirstNotInserted = FALSE;
				}
				InsertOneSigalNode_i(sigInfo2);
				++nGetCount;
				if ((nGetCount & 0xF) == 0 && 
					CQtTimeValue::GetTimeOfDay() - tvCurOld >= CQtTimeValue(0, 1*1000))
				{
					QT_INFO_TRACE_THIS("CQtReactorRealTimeSignalBuffer::RunEventLoop,"
						" nGetCount=" << nGetCount);
					break;
				}
			}
			else {
				break;
			}
		}

		QtResult rv;
		if (bFirstNotInserted) {
			if (sigInfo.si_code == SI_QUEUE)
				rv = ProcessOneSignalQueue(sigInfo.si_value.sival_int);
			else
				rv = ProcessOneSignalFd(sigInfo.si_fd, sigInfo.si_band);
			QT_ASSERTE(m_SigalNodes.empty());
		}
		else {
			SigalNodesType::iterator iter = m_SigalNodes.begin();
			for ( ; iter != m_SigalNodes.end(); ++iter) {
				if (iter->m_si_code == SI_QUEUE)
					rv = ProcessOneSignalQueue(iter->m_sival_int);
				else {
					rv = QT_OK;
					if (iter->m_CodeIo.m_si_band & POLLIN) {
						QT_ASSERTE(iter->m_CodeIo.m_dwReadCount > 0);
						for (DWORD i = 0; i < iter->m_CodeIo.m_dwReadCount; ++i) {
							rv = ProcessHandleEvent(
								iter->m_CodeIo.m_si_fd, 
								AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK, 
								QT_OK, 
								FALSE);
							if (QT_FAILED(rv))
								break;
						}
						if (QT_FAILED(rv) && rv != QT_ERROR_WOULD_BLOCK)
							continue;
					}
					if (iter->m_CodeIo.m_si_band & POLLOUT) {
						rv = ProcessHandleEvent(
							iter->m_CodeIo.m_si_fd, 
							AQtEventHandler::WRITE_MASK | AQtEventHandler::CONNECT_MASK, 
							QT_OK, 
							FALSE);
						if (QT_FAILED(rv) && rv != QT_ERROR_WOULD_BLOCK)
							continue;
					}
					if (iter->m_CodeIo.m_si_band & (POLLERR|POLLHUP|POLLNVAL)) {
						QT_WARNING_TRACE_THIS("CQtReactorRealTimeSignalBuffer::RunEventLoop,"
							" handle is closed."
							" fd=" << iter->m_CodeIo.m_si_fd <<
							" band=" << iter->m_CodeIo.m_si_band);
						rv = ProcessHandleEvent(
							iter->m_CodeIo.m_si_fd, 
							AQtEventHandler::CLOSE_MASK, 
							QT_ERROR_NETWORK_SOCKET_CLOSE, 
							FALSE);
					}
				}
			}
			m_SigalNodes.clear();
			::memset(m_pSigalNodesIndex, 0, sizeof(CSigalNode *) * m_nSigalNodeIndexNumber);
		}

		if (sigRet == SIGIO) 
			ProcessSignalOverFlow();
	}
	return QT_OK;
}
