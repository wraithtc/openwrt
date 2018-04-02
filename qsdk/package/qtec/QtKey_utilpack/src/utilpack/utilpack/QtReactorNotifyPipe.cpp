
#include "QtBase.h"
#include "QtReactorNotifyPipe.h"
#include "QtReactorBase.h"
#include "QtSocket.h"

CQtReactorNotifyPipe::CQtReactorNotifyPipe()
	: m_pReactor(NULL)
{
}

CQtReactorNotifyPipe::~CQtReactorNotifyPipe()
{
	Close();
}

QtResult CQtReactorNotifyPipe::Open(CQtReactorBase *aReactor)
{
	QtResult rv = QT_OK;
	CQtIPCBase ipcNonblock;
	
	QT_ASSERTE(!m_pReactor);
	m_pReactor = aReactor;
	QT_ASSERTE_RETURN(m_pReactor, QT_ERROR_INVALID_ARG);

	rv = m_PipeNotify.Open();
	if (QT_FAILED(rv)) 
		goto fail;

	ipcNonblock.SetHandle(m_PipeNotify.GetReadHandle());
	if (ipcNonblock.Enable(CQtIPCBase::NON_BLOCK) == -1) {
		QT_ERROR_TRACE_THIS("CQtReactorNotifyPipe::Open, Enable(NON_BLOCK) failed! err=" << errno);
		rv = QT_ERROR_NETWORK_SOCKET_ERROR;
		goto fail;
	}
	
	rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK);
	if (QT_FAILED(rv)) 
		goto fail;
	
	QT_INFO_TRACE_THIS("CQtReactorNotifyPipe::Open,"
		" read_fd=" << m_PipeNotify.GetReadHandle() << " write_fd=" << m_PipeNotify.GetWriteHandle());
	return QT_OK;

fail:
	Close();
	QT_ASSERTE(QT_FAILED(rv));
	return rv;
}

QT_HANDLE CQtReactorNotifyPipe::GetHandle() const 
{
	return m_PipeNotify.GetReadHandle();
}

int CQtReactorNotifyPipe::OnInput(QT_HANDLE aFd)
{
	QT_ASSERTE(aFd == m_PipeNotify.GetReadHandle());
	
	CBuffer bfNew;
	int nRecv = ::recv(
		(QT_SOCKET)m_PipeNotify.GetReadHandle(), 
		(char*)&bfNew, sizeof(bfNew), 0);

	if (nRecv < (int)sizeof(bfNew)) {
#ifdef QT_WIN32
		errno = ::WSAGetLastError();
#endif // QT_WIN32
		QT_ERROR_TRACE_THIS("CQtReactorNotifyPipe::OnInput,"
			" nRecv=" << nRecv <<
			" fd=" << m_PipeNotify.GetReadHandle() << 
			" err=" << errno);

		return 0;
	}

	// we use sigqueue to notify close
	// so that we needn't this pipi to stop the reactor.
#if 0
	BOOL bStopReactor = FALSE;
	if (bfNew.m_Fd == m_PipeNotify.GetReadHandle()) {
		QT_ASSERTE(bfNew.m_Mask == AQtEventHandler::CLOSE_MASK);
		bfNew.m_Fd = QT_INVALID_HANDLE;
		bStopReactor = TRUE;
	}
#else
	if (bfNew.m_Fd == m_PipeNotify.GetReadHandle())
		return 0;
#endif

	QT_ASSERTE(m_pReactor);
	if (m_pReactor)
		m_pReactor->ProcessHandleEvent(bfNew.m_Fd, bfNew.m_Mask, QT_OK, TRUE);

#if 0
	if (bStopReactor) {
		QT_INFO_TRACE_THIS("CQtReactorNotifyPipe::OnInput, reactor is stopped.");
		m_pReactor->CQtStopFlag::SetStopFlag();
	}
#endif
	return 0;
}

QtResult CQtReactorNotifyPipe::
Notify(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	// this function can be invoked in the different thread.
	if (m_PipeNotify.GetWriteHandle() == QT_INVALID_HANDLE) {
		QT_WARNING_TRACE_THIS("CQtReactorNotifyPipe::Notify, WriteHandle INVALID.");
		return QT_ERROR_NOT_INITIALIZED;
	}

	QT_HANDLE fdNew = QT_INVALID_HANDLE;
	if (aEh) {
		fdNew = aEh->GetHandle();
		QT_ASSERTE(fdNew != QT_INVALID_HANDLE);
	}
	
	CBuffer bfNew(fdNew, aMask);
	int nSend = ::send(
		(QT_SOCKET)m_PipeNotify.GetWriteHandle(), 
		(char*)&bfNew, sizeof(bfNew), 0);
	if (nSend < (int)sizeof(bfNew)) {
		QT_ERROR_TRACE_THIS("CQtReactorNotifyPipe::Notify,"
			" nSend=" << nSend <<
			" fd=" << m_PipeNotify.GetWriteHandle() <<
			" err=" << errno);
		return QT_ERROR_UNEXPECTED;
	}
	return QT_OK;
}

QtResult CQtReactorNotifyPipe::Close()
{
	if (m_pReactor) {
		m_pReactor->RemoveHandler(this);
		m_pReactor = NULL;
	}
	return m_PipeNotify.Close();
}

