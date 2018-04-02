
#include "QtBase.h"
#include "QtReactorSelect.h"
#include "QtTimerQueueBase.h"

CQtReactorSelect::CQtReactorSelect()
	: CQtReactorBase(SEND_REGISTER_PROPERTY)
{
}

CQtReactorSelect::~CQtReactorSelect()
{
}

QtResult CQtReactorSelect::Open()
{
	QtResult rv = CQtReactorBase::Open();
	if (QT_FAILED(rv))
		goto fail;

	rv = m_Notify.Open(this);
	if (QT_FAILED(rv))
		goto fail;

	CQtStopFlag::SetStartFlag();
	QT_INFO_TRACE_THIS("CQtReactorSelect::Open()");
	return QT_OK;

fail:
	Close();
	QT_ASSERTE(QT_FAILED(rv));
	QT_ERROR_TRACE_THIS("CQtReactorSelect::Open, failed!"
		" rv=" << rv);
	return rv;
}

QtResult CQtReactorSelect::
NotifyHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	return m_Notify.Notify(aEh, aMask);
}

QtResult CQtReactorSelect::RunEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorSelect::RunEventLoop");
	m_Est.EnsureSingleThread();

	while (!CQtStopFlag::IsFlagStopped()) {
		CQtTimeValue tvTimeout(CQtTimeValue::s_tvMax);
		if (m_pTimerQueue) {
			// process timer prior to wait event.
			m_pTimerQueue->CheckExpire(&tvTimeout);
		}

		timeval tvSelect;
		tvSelect.tv_sec = tvTimeout.GetSec();
		tvSelect.tv_usec = tvTimeout.GetUsec();

		fd_set fsRead, fsWrite, fsException;
		FD_ZERO(&fsRead);
		FD_ZERO(&fsWrite);
		FD_ZERO(&fsException);
		int nMaxFd = m_EhRepository.FillFdSets(fsRead, fsWrite, fsException);
		QT_ASSERTE(nMaxFd >= 0);

	#ifdef QT_MACOS
	#ifndef MachOSupport
	 	int nSelect = ::CFM_select(nMaxFd+1, &fsRead, &fsWrite, &fsException, 
	  	tvTimeout == CQtTimeValue::s_tvMax ? NULL : &tvSelect);
	  	if (nSelect == 0 || (nSelect == -1 && CFM_geterrno() == EINTR))
	#else
		int nSelect = ::select(nMaxFd+1, &fsRead, &fsWrite, &fsException, 
	  	tvTimeout == CQtTimeValue::s_tvMax ? NULL : &tvSelect);
	  	if (nSelect == 0 || (nSelect == -1 && errno == EINTR))
	#endif	//MachOSupport  	
	#else
		int nSelect = ::select(nMaxFd+1, &fsRead, &fsWrite, &fsException, 
			(tvTimeout == CQtTimeValue::s_tvMax ? NULL : &tvSelect));
		if (nSelect == 0 || (nSelect == -1 && errno == EINTR))
	#endif	
			continue;
		else if (nSelect == -1) {
			QT_ERROR_TRACE_THIS("CQtReactorSelect::RunEventLoop, select() failed!"
				" nMaxFd=" << nMaxFd << " err=" << errno);
			return QT_ERROR_FAILURE;
		}

		int nActiveNumber = nSelect;
		ProcessFdSets_i(
			fsRead, 
			AQtEventHandler::READ_MASK | AQtEventHandler::ACCEPT_MASK | AQtEventHandler::CONNECT_MASK, 
			nActiveNumber, nMaxFd);
		ProcessFdSets_i(
			fsWrite, 
			AQtEventHandler::WRITE_MASK | AQtEventHandler::CONNECT_MASK, 
			nActiveNumber, nMaxFd);
#ifdef QT_WIN32
		ProcessFdSets_i(
			fsException, 
			AQtEventHandler::CLOSE_MASK,
			nActiveNumber, nMaxFd);
#endif // QT_WIN32

		// Needn't check nActiveNumber due to because 
		// fd maybe removed when doing callback.
//		QT_ASSERTE(nActiveNumber == 0);
	}
	return QT_OK;
}

void CQtReactorSelect::
ProcessFdSets_i(fd_set &aFdSet, AQtEventHandler::MASK aMask, 
				int &aActiveNumber, int aMaxFd)
{
#ifdef QT_WIN32
	for (unsigned i = 0; i < aFdSet.fd_count && aActiveNumber > 0; i++) {
		QT_HANDLE fdGet = (QT_HANDLE)aFdSet.fd_array[i];
		if (fdGet == QT_INVALID_HANDLE)
			continue;
		aActiveNumber--;
		ProcessHandleEvent(fdGet, aMask, QT_OK, FALSE);
	}
#else
	CQtEventHandlerRepository::CElement* pElements = m_EhRepository.GetElement();
	for (int i = 0; i <= aMaxFd; i++) {
		CQtEventHandlerRepository::CElement &eleGet = pElements[i];
		if (!eleGet.IsCleared()) {
//			QT_HANDLE fdGet = eleGet.m_pEh->GetHandle();
			if (FD_ISSET(i, &aFdSet)) {
				aActiveNumber--;
				ProcessHandleEvent(i, aMask, QT_OK, FALSE);
			}
		}
	}
#endif // QT_WIN32
}

QtResult CQtReactorSelect::StopEventLoop()
{
//	CQtStopFlag::SetStopFlag();
	CQtStopFlag::m_bStoppedFlag = TRUE;
	m_Notify.Notify(&m_Notify, AQtEventHandler::NULL_MASK);
	return QT_OK;
}

QtResult CQtReactorSelect::Close()
{
	m_Notify.Close();
	return CQtReactorBase::Close();
}

QtResult CQtReactorSelect::
OnHandleRegister(QT_HANDLE aFd, AQtEventHandler::MASK aMask, AQtEventHandler *aEh)
{
	return QT_OK;
}

void CQtReactorSelect::OnHandleRemoved(QT_HANDLE aFd)
{
}
