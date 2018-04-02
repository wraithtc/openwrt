
#include "QtBase.h"
#include "QtTransportTcp.h"
#include "QtInetAddr.h"
#include "QtMessageBlock.h"
#include "QtReactorInterface.h"
#include "QtThreadInterface.h"

CQtTransportTcp::CQtTransportTcp(IQtReactor *pReactor)
	: CQtTransportBase(pReactor)
	, m_bNeedOnSend(FALSE)
{
	m_pThread = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
	QT_ASSERTE(m_pThread);
}

CQtTransportTcp::~CQtTransportTcp()
{
	Close_t(QT_OK);
}

QtResult CQtTransportTcp::Open_t()
{
	QtResult rv = QT_ERROR_NETWORK_SOCKET_ERROR;
	
	DWORD dwRcv = 65535, dwSnd = 65535;
	rv = SetOption(QT_OPT_TRANSPORT_RCV_BUF_LEN, &dwRcv);
//	QT_ASSERTE(QT_SUCCEEDED(rv));
	rv = SetOption(QT_OPT_TRANSPORT_SND_BUF_LEN, &dwSnd);
//	QT_ASSERTE(QT_SUCCEEDED(rv));

	int nNoDelay = 1;
	if (m_SocketTcp.SetOption(IPPROTO_TCP, TCP_NODELAY, &nNoDelay, sizeof(nNoDelay)) == -1) {
		QT_ERROR_TRACE_THIS("CQtTransportTcp::Open_t, SetOption(TCP_NODELAY) failed!"
			" err=" << errno);
	}

	if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY) {
		rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK);
	}
	else {
		// it works with Win32AsyncSelect and RealTimeSignal 
		// if we regiests READ_MASK & WRITE_MASK together at first
		rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
	}

	if (QT_FAILED(rv) && rv != QT_ERROR_FOUND) {
		QT_ERROR_TRACE_THIS("CQtTransportTcp::Open_t, RegisterHandler(READ_MASK|WRITE_MASK) failed!");
		return rv;
	}
	else
		return QT_OK;
}

QT_HANDLE CQtTransportTcp::GetHandle() const
{
	return m_SocketTcp.GetHandle();
}


int CQtTransportTcp::OnInput(QT_HANDLE )
{
	//for signal combine issue, need a event to read the data remain in the receiver buffer
	//5/8 2009 Victor
	//disable them now, maybe we open them in the another train release to avoid risk
/*	class CTransportTcpOnInputEvent: public IQtEvent
	{
		//we can not use autoptr here, because we can not sure that upper layer use it or not
		CQtTransportTcp * m_pThransport; 
	public:
		CTransportTcpOnInputEvent(CQtTransportTcp *pTransport)
			:m_pThransport(pTransport)
		{
			m_EventType = EVENT_TCP_ONINPUT;
		}
		QtResult OnEventFire()
		{
			if(m_pThransport)
				m_pThransport->OnInput();
			return QT_OK;
		}
		
	};*/
	//////////////////////////////////////////////////////////////////////////
	static char szBuf[16 * 1024];
	int nRecv = Recv_i(szBuf, sizeof(szBuf));
	
	if (nRecv <= 0)
		return nRecv;

	CQtMessageBlock mbOn(
		nRecv, 
		szBuf, 
		CQtMessageBlock::DONT_DELETE | CQtMessageBlock::WRITE_LOCKED, 
		nRecv);
	
	QT_ASSERTE(m_pSink);
	if (m_pSink)
		m_pSink->OnReceive(mbOn, this);

	if (nRecv < static_cast<int>(sizeof(szBuf)))
		return -2;
//////////////////////////////////////////////////////////////////////////
	//disable them now, maybe we open them in the another train release to avoid risk
/*
	else if (nRecv == static_cast<int>(sizeof(szBuf))) //should be has some data remain in the buffer
	{
		IQtEventQueue  *pQueue = m_pThread ? m_pThread->GetEventQueue() : NULL;
		if(!pQueue) //have no queue
			return -1;
		pQueue->PostEvent(new CTransportTcpOnInputEvent(this));
		return QT_OK;
	}
*/
//////////////////////////////////////////////////////////////////////////
	else
		return 0;
}

int CQtTransportTcp::OnOutput(QT_HANDLE )
{
//	QT_INFO_LOG(("CQtTransportTcp::OnOutput"));

	if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY) {
		QT_ASSERTE(m_bNeedOnSend);
		m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK);
	} 
	else if (!m_bNeedOnSend) 
		return 0;
	m_bNeedOnSend = FALSE;

	QT_ASSERTE(m_pSink);
	if (m_pSink) 
		m_pSink->OnSend(this);
	return 0;
}

CQtSocketTcp& CQtTransportTcp::GetPeer()
{
	return m_SocketTcp;
}

QtResult CQtTransportTcp::Close_t(QtResult aReason)
{
	if (m_SocketTcp.GetHandle() != QT_INVALID_HANDLE) {
		m_pReactor->RemoveHandler(this);
		m_SocketTcp.Close(aReason);
	}
	return QT_OK;
}

QtResult CQtTransportTcp::
SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{
	if (aPara)
		aPara->m_dwHaveSent = 0;

	if (m_SocketTcp.GetHandle() == QT_INVALID_HANDLE) {
		QT_WARNING_TRACE_THIS("CQtTransportTcp::SendData, socket is invalid.");
		return QT_ERROR_NOT_INITIALIZED;
	}
	
	static iovec iov[QT_IOV_MAX];
	DWORD dwFill = aData.FillIov(iov, QT_IOV_MAX);
	QT_ASSERTE_RETURN(dwFill > 0, QT_OK);

	int nSend = m_SocketTcp.SendV(iov, dwFill);
	if (nSend < 0) {
		if (errno == EWOULDBLOCK) {
			if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY)
				m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
			m_bNeedOnSend = TRUE;
			return QT_ERROR_PARTIAL_DATA;
		}
		else {
			QT_WARNING_TRACE_THIS("CQtTransportTcp::SendData, sendv() failed! err=" << errno);
			// Don't NotifyHandler avoid blocking due to otify pipe overflow.
//			if (m_pReactor)
//				m_pReactor->NotifyHandler(this, AQtEventHandler::CLOSE_MASK);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
	}

	if (aPara)
		aPara->m_dwHaveSent = nSend;

	aData.AdvanceChainedReadPtr(nSend);
	if (aData.GetChainedLength()) {
		if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY)
			m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
		m_bNeedOnSend = TRUE;
		return QT_ERROR_PARTIAL_DATA;
	}
	else
		return QT_OK;
}

QtResult CQtTransportTcp::GetOption(DWORD aCommand, LPVOID aArg)
{
	switch (aCommand) {
	case QT_OPT_TRANSPORT_FIO_NREAD:
		if (m_SocketTcp.Control(FIONREAD, aArg) == -1) {
			QT_WARNING_TRACE_THIS("CQtTransportTcp::GetOption, (QT_OPT_TRANSPORT_FIO_NREAD) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		return QT_OK;

	case QT_OPT_TRANSPORT_FD:
		*(static_cast<QT_HANDLE *>(aArg)) = m_SocketTcp.GetHandle();
		return QT_OK;

	case QT_OPT_TRANSPORT_LOCAL_ADDR:
		if (m_SocketTcp.GetLocalAddr(*(static_cast<CQtInetAddr*>(aArg))) == -1) {
			QT_WARNING_TRACE_THIS("CQtTransportTcp::GetOption, (QT_OPT_TRANSPORT_LOCAL_ADDR) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_PEER_ADDR:
		if (m_SocketTcp.GetRemoteAddr(*(static_cast<CQtInetAddr*>(aArg))) == -1) {
			QT_WARNING_TRACE_THIS("CQtTransportTcp::GetOption, (QT_OPT_TRANSPORT_PEER_ADDR) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_SOCK_ALIVE: {
		if (m_SocketTcp.GetHandle() == QT_INVALID_HANDLE) {
			*static_cast<BOOL*>(aArg) = FALSE;
			return QT_ERROR_NOT_INITIALIZED;
		}
		char cTmp;
		int nRet = m_SocketTcp.Recv(&cTmp, sizeof(cTmp), MSG_PEEK);
		if (nRet > 0 || (nRet < 0 && errno == EWOULDBLOCK))
			*static_cast<BOOL*>(aArg) = TRUE;
		else
			*static_cast<BOOL*>(aArg) = FALSE;
		return QT_OK;
	}
		
	case QT_OPT_TRANSPORT_TRAN_TYPE:
		*(static_cast<CQtConnectionManager::CType*>(aArg)) 
			= CQtConnectionManager::CTYPE_TCP;
		return QT_OK;

	case QT_OPT_TRANSPORT_RCV_BUF_LEN: {
		int nLen = sizeof(DWORD);
		if (m_SocketTcp.GetOption(SOL_SOCKET, SO_RCVBUF, aArg, &nLen) == -1) {
//			QT_ERROR_TRACE_THIS("CQtTransportTcp::GetOption, GetOption(SO_RCVBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;
										}

	case QT_OPT_TRANSPORT_SND_BUF_LEN: {
		int nLen = sizeof(DWORD);
		if (m_SocketTcp.GetOption(SOL_SOCKET, SO_SNDBUF, aArg, &nLen) == -1) {
//			QT_ERROR_TRACE_THIS("CQtTransportTcp::GetOption, GetOption(SO_SNDBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;
									   }
	case QT_OPT_TRANSPORT_TOS: 
		return QT_OK;
/*		{
		int nLen = sizeof(DWORD);
		if (m_SocketTcp.GetOption(IPPROTO_IP, IP_TOS, aArg, &nLen) == -1) {
			//			QT_ERROR_TRACE_THIS("CQtTransportTcp::GetOption, GetOption(SO_SNDBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;
									   }
*/
	default:
		QT_WARNING_TRACE_THIS("CQtTransportTcp::GetOption,"
			" unknow aCommand=" << aCommand << 
			" aArg=" << aArg);
		return QT_ERROR_INVALID_ARG;
	}
	return QT_ERROR_FAILURE;
}

QtResult CQtTransportTcp::SetOption(DWORD aCommand, LPVOID aArg)
{
	QT_ASSERTE_RETURN(aArg, QT_ERROR_INVALID_ARG);
	switch (aCommand) {
	case QT_OPT_TRANSPORT_FD: {
		// we allow user to set TCP socket to QT_INVALID_HANDLE, 
		// mainly used by CQtConnectorProxyT.
		QT_HANDLE hdNew = *(static_cast<QT_HANDLE *>(aArg));
		QT_ASSERTE_RETURN(hdNew == QT_INVALID_HANDLE, QT_ERROR_INVALID_ARG);
		m_SocketTcp.SetHandle(hdNew);
		return QT_OK;
							  }

	case QT_OPT_TRANSPORT_RCV_BUF_LEN:
		if (m_SocketTcp.SetOption(SOL_SOCKET, SO_RCVBUF, aArg, sizeof(DWORD)) == -1) {
//			QT_ERROR_TRACE_THIS("CQtTransportTcp::SetOption, SetOption(SO_RCVBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_SND_BUF_LEN:
		if (m_SocketTcp.SetOption(SOL_SOCKET, SO_SNDBUF, aArg, sizeof(DWORD)) == -1) {
//			QT_ERROR_TRACE_THIS("CQtTransportTcp::SetOption, SetOption(SO_SNDBUF) failed! err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
		else
			return QT_OK;

	case QT_OPT_TRANSPORT_TCP_KEEPALIVE: {
		DWORD dwTime = *static_cast<DWORD*>(aArg);
		int nKeep = dwTime > 0 ? 1 : 0;
		if (m_SocketTcp.SetOption(SOL_SOCKET, SO_KEEPALIVE, &nKeep, sizeof(nKeep)) == -1) {
			QT_ERROR_TRACE_THIS("CQtTransportTcp::SetOption, SetOption(SO_KEEPALIVE) failed!"
				" dwTime=" << dwTime << 
				" err=" << errno);
			return QT_ERROR_NETWORK_SOCKET_ERROR;
		}
#ifdef QT_LINUX
		if (dwTime > 0) {
			if (m_SocketTcp.SetOption(SOL_TCP, TCP_KEEPIDLE, &dwTime, sizeof(dwTime)) == -1) {
				QT_ERROR_TRACE_THIS("CQtTransportTcp::SetOption, SetOption(TCP_KEEPINTVL) failed!"
					" dwTime=" << dwTime << 
					" err=" << errno);
				return QT_ERROR_NETWORK_SOCKET_ERROR;
			}
		}
#endif // QT_LINUX
		return QT_OK;
	}

	case QT_OPT_TRANSPORT_TOS: 
		return SetTos2Socket(m_SocketTcp, aArg);

	default:
		QT_WARNING_TRACE_THIS("CQtTransportTcp::SetOption,"
			" unknow aCommand=" << aCommand << 
			" aArg=" << aArg);
		return QT_ERROR_INVALID_ARG;
	}
}
