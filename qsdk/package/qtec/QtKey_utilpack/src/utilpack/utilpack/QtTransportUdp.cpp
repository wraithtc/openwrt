
#include "QtBase.h"
#include "QtTransportUdp.h"
#include "QtInetAddr.h"
#include "QtAcceptorUdp.h"

CQtTransportUdp::CQtTransportUdp(IQtReactor *pReactor, const CQtInetAddr &aAddrSend
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
								 , CQtAcceptorUdp *pAcceptor
#endif
								 )
	: CQtTransportBase(pReactor)
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
	, m_pAcceptor(pAcceptor)
#endif
	, m_AddrSend(aAddrSend)
{
}

CQtTransportUdp::~CQtTransportUdp()
{
	Close_t(QT_OK);
}

QtResult CQtTransportUdp::Open_t()
{
	QtResult rv = QT_OK;
	// Don't RegisterHandler if it is created by Acceptor because Acceptor has registered it.
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
	if (m_pAcceptor) 
		return rv;
#endif
	DWORD dwRcv = 65535, dwSnd = 65535;
	rv = SetOption(QT_OPT_TRANSPORT_RCV_BUF_LEN, &dwRcv);
	QT_ASSERTE(QT_SUCCEEDED(rv));
	rv = SetOption(QT_OPT_TRANSPORT_SND_BUF_LEN, &dwSnd);
	QT_ASSERTE(QT_SUCCEEDED(rv));

	rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::UDP_LINK_MASK);
	if (QT_FAILED(rv) && rv != QT_ERROR_FOUND) {
		QT_ERROR_TRACE_THIS("CQtTransportUdp::Open_t, RegisterHandler(READ_MASK) failed! rv=" << rv);
		return rv;
	}
	else {
		return QT_OK;
	}
}

QT_HANDLE CQtTransportUdp::GetHandle() const 
{
	return m_SocketUdp.GetHandle();
}

int CQtTransportUdp::OnInput(QT_HANDLE )
{
	static char szBuf[CQtConnectionManager::UDP_SEND_MAX_LEN];
	int nRecv = m_SocketUdp.Recv(szBuf, sizeof(szBuf));
	if (nRecv <= 0) {
		int nErr = errno;
		QT_WARNING_TRACE_THIS("CQtTransportUdp::OnInput, Recv() failed!"
			" nRecv=" << nRecv
/*
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
			<< " m_pAcceptor=" << m_pAcceptor <<
#endif
*/
			<< " err=" << nErr);

		// don't return -2 to avoid OnInput() again every time receiving the signal.
		if (nErr == EWOULDBLOCK)
			return 0;

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
		QT_ASSERTE(!m_pAcceptor);
#endif
		return -1;
	}

	OnReceiveCallback(szBuf, nRecv);
	return 0;
}

CQtSocketUdp& CQtTransportUdp::GetPeer()
{
	return m_SocketUdp;
}

QtResult CQtTransportUdp::Close_t(QtResult aReason)
{
/*	QT_INFO_TRACE_THIS("CQtTransportUdp::Close_t,"
		" m_pAcceptor=" << m_pAcceptor << 
		" fd=" << m_SocketUdp.GetHandle() << 
		" aReason=" << aReason << 
		" ip=" << m_AddrSend.GetIpDisplayName() << 
		" port=" << m_AddrSend.GetPort());
*/
	// Don't close socket if it is created by Acceptor.
	if (m_SocketUdp.GetHandle() != QT_INVALID_HANDLE) {
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
		if (!m_pAcceptor) 
#endif
		{
			m_pReactor->RemoveHandler(this);
			m_SocketUdp.Close();
		}
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
		else {
			m_pAcceptor->RemoveTransport(m_AddrSend, this);
			m_SocketUdp.SetHandle(QT_INVALID_HANDLE);
		}
#endif
	}
	return QT_OK;
}

QtResult CQtTransportUdp::
SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{
	QT_ASSERTE(aData.GetChainedLength() <= CQtConnectionManager::UDP_SEND_MAX_LEN);
	if (aPara)
		aPara->m_dwHaveSent = 0;

	if (m_SocketUdp.GetHandle() == QT_INVALID_HANDLE) {
		QT_WARNING_TRACE_THIS("CQtTransportUdp::SendData, socket is invalid.");
		return QT_ERROR_NOT_INITIALIZED;
	}

	static iovec iov[QT_IOV_MAX];
	DWORD dwFill = aData.FillIov(iov, QT_IOV_MAX);
	QT_ASSERTE_RETURN(dwFill > 0, QT_OK);

	// ensure iovec are filled with all chained <CQtMessageBlock>s.
	QT_ASSERTE(dwFill < QT_IOV_MAX);

	// SendVTo() will failed if after connect() in Win98.
	int nSend;
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
	if (m_pAcceptor)
		nSend = m_SocketUdp.SendVTo(iov, dwFill, m_AddrSend);
	else 
#endif
		nSend = m_SocketUdp.SendV(iov, dwFill);
	if (nSend < (int)aData.GetChainedLength()) {
		QT_WARNING_TRACE_THIS("CQtTransportUdp::SendData, sendv() failed!"
			" nSend=" << nSend <<
			" send_len=" << aData.GetChainedLength() <<
			" dwFill=" << dwFill <<
			" addr=" << m_AddrSend.GetIpDisplayName() <<
			" port=" << m_AddrSend.GetPort() <<
			" err=" << errno);

#ifdef QT_MACOS
		if(m_pSink) m_pSink->OnDisconnect(QT_ERROR_NETWORK_SOCKET_ERROR, this);
#endif
		// Don't need this assert 
//		QT_ASSERTE(!m_pAcceptor);

		// Don't close this transport because we allow send failed.
//		QT_ASSERTE(m_pReactor);
//		if (m_pReactor)
//			m_pReactor->NotifyHandler(this, AQtEventHandler::CLOSE_MASK);

		QT_ASSERTE(nSend <= 0);
		// And don't return QT_ERROR_NETWORK_SOCKET_ERROR;
		// because CQtTransportThreadProxy will wait OnDisconnect().
		return QT_ERROR_PARTIAL_DATA;
	}
	
	if (aPara)
		aPara->m_dwHaveSent = nSend;
	aData.AdvanceChainedReadPtr(nSend);
	QT_ASSERTE(aData.GetChainedLength() == 0);
	return QT_OK;
}

QtResult CQtTransportUdp::GetOption(DWORD aCommand, LPVOID aArg)
{
	QT_ASSERTE_RETURN(aArg, QT_ERROR_INVALID_ARG);
	switch (aCommand) {
	case QT_OPT_TRANSPORT_FIO_NREAD:
		if (m_SocketUdp.Control(FIONREAD, aArg) == -1) {
			QT_WARNING_TRACE_THIS("CQtTransportUdp::GetOption, (QT_OPT_TRANSPORT_FIO_NREAD) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		return QT_OK;

	case QT_OPT_TRANSPORT_FD:
		*(static_cast<QT_HANDLE *>(aArg)) = m_SocketUdp.GetHandle();
		return QT_OK;

	case QT_OPT_TRANSPORT_LOCAL_ADDR:
		if (m_SocketUdp.GetLocalAddr(*(static_cast<CQtInetAddr*>(aArg))) == -1) {
			QT_WARNING_TRACE_THIS("CQtTransportUdp::GetOption, (QT_OPT_TRANSPORT_LOCAL_ADDR) failed!"
				" fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_PEER_ADDR:
		*(static_cast<CQtInetAddr*>(aArg)) = m_AddrSend;
		return QT_OK;

	case QT_OPT_TRANSPORT_TRAN_TYPE:
		*(static_cast<CQtConnectionManager::CType*>(aArg)) = CQtConnectionManager::CTYPE_UDP;
		return QT_OK;

	case QT_OPT_TRANSPORT_SND_BUF_LEN: 
		{
		int nLen = sizeof(DWORD);
		if (m_SocketUdp.GetOption(SOL_SOCKET, SO_SNDBUF, aArg, &nLen) == -1) {
			QT_ERROR_TRACE_THIS("CQtTransportUdp::GetOption, GetOption(SO_SNDBUF) failed!"
				" fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;
									   }

	case QT_OPT_TRANSPORT_RCV_BUF_LEN: 
		{
		int nLen = sizeof(DWORD);
		if (m_SocketUdp.GetOption(SOL_SOCKET, SO_RCVBUF, aArg, &nLen) == -1) {
			QT_ERROR_TRACE_THIS("CQtTransportUdp::GetOption, GetOption(SO_RCVBUF) failed!"
				" fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;
									   }

	case QT_OPT_TRANSPORT_SOCK_ALIVE: 
		{
		if (m_SocketUdp.GetHandle() == QT_INVALID_HANDLE) {
			*static_cast<BOOL*>(aArg) = FALSE;
			return QT_ERROR_NOT_INITIALIZED;
		}
		else {
			*static_cast<BOOL*>(aArg) = TRUE;
			return QT_OK;
		}
									  }

	default:
		QT_WARNING_TRACE_THIS("CQtTransportUdp::GetOption,"
			" unknow aCommand=" << aCommand << 
			" aArg=" << aArg);
		return QT_ERROR_INVALID_ARG;
	}
}

QtResult CQtTransportUdp::SetOption(DWORD aCommand, LPVOID aArg)
{
	QT_ASSERTE_RETURN(aArg, QT_ERROR_INVALID_ARG);

	switch (aCommand) {
	case QT_OPT_TRANSPORT_SND_BUF_LEN:
		if (m_SocketUdp.SetOption(SOL_SOCKET, SO_SNDBUF, aArg, sizeof(DWORD)) == -1) {
			QT_ERROR_TRACE_THIS("CQtTransportUdp::SetOption, SetOption(SO_SNDBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_RCV_BUF_LEN:
		if (m_SocketUdp.SetOption(SOL_SOCKET, SO_RCVBUF, aArg, sizeof(DWORD)) == -1) {
			QT_ERROR_TRACE_THIS("CQtTransportUdp::SetOption, SetOption(SO_RCVBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_TOS: 
		return SetTos2Socket(m_SocketUdp, aArg);

	default:
		QT_WARNING_TRACE_THIS("CQtTransportUdp::SetOption,"
			" unknow aCommand=" << aCommand << 
			" aArg=" << aArg);
		return QT_ERROR_INVALID_ARG;
	}
}
