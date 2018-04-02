
#include "QtBase.h"
#include "QtAcceptorTcp.h"
#include "QtInetAddr.h"
#include "QtTransportTcp.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

CQtAcceptorTcp::CQtAcceptorTcp()
{
}

CQtAcceptorTcp::~CQtAcceptorTcp()
{
	StopListen(QT_OK);
}

QtResult CQtAcceptorTcp::
StartListen(IQtAcceptorConnectorSink *aSink, const CQtInetAddr &aAddrListen, int nTraceInterval)
{
	QtResult rv = QT_ERROR_NETWORK_SOCKET_ERROR;
	int nOption;
	QT_ASSERTE_RETURN(m_Socket.GetHandle() == QT_INVALID_HANDLE, QT_ERROR_ALREADY_INITIALIZED);

	QT_ASSERTE(!m_pSink);
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
	m_pSink = aSink;
	m_Interval = nTraceInterval;
	m_AcceptCount = 0;
	/// win2000 will allow 2 processes listening on the same port 
	/// if the socket's option allows ReuseAddress.
#ifdef WIN32
	BOOL bReuseAddr = FALSE;
#else // !WIN32
	BOOL bReuseAddr = TRUE;
#endif // WIN32
	int nRet = m_Socket.Open(bReuseAddr);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtAcceptorTcp::StartListen, Open() failed!"
			" addr=" << aAddrListen.GetIpDisplayName() <<
			" port=" << aAddrListen.GetPort() <<
			" err=" << errno);
		goto fail;
	}

	nRet = ::bind(
		(QT_SOCKET)m_Socket.GetHandle(), 
		reinterpret_cast<const struct sockaddr *>(aAddrListen.GetPtr()), 
		aAddrListen.GetSize());
#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // ! QT_WIN32
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtAcceptorTcp::StartListen, bind() failed!"
			" addr=" << aAddrListen.GetIpDisplayName() <<
			" port=" << aAddrListen.GetPort() <<
			" err=" << errno);
		rv = QT_ERROR_NETWORK_SOCKET_ERROR;
		goto fail;
	}

	nOption = m_Socket.SetOption(SOL_SOCKET, SO_SNDBUF,  &m_nSndBuffLen, sizeof(DWORD));
	QT_ASSERTE(nOption == 0);
	nOption = m_Socket.SetOption(SOL_SOCKET, SO_RCVBUF,  &m_nRcvBuffLen, sizeof(DWORD));
	
	nRet = ::listen((QT_SOCKET)m_Socket.GetHandle(), 1024/*SOMAXCONN*/);
#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // ! QT_WIN32
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtAcceptorTcp::StartListen, listen() failed! err=" << errno);
		rv = QT_ERROR_NETWORK_SOCKET_ERROR;
		goto fail;
	}

	rv = m_pReactor->RegisterHandler(this, AQtEventHandler::ACCEPT_MASK);
	if (QT_FAILED(rv))
		goto fail;

	QT_INFO_TRACE_THIS("CQtAcceptorTcp::StartListen,"
		" addr=" << aAddrListen.GetIpDisplayName() <<
		" port=" << aAddrListen.GetPort() << 
		" aSink=" << aSink << 
		" fd=" << m_Socket.GetHandle() <<
		" Trace interval = " << m_Interval);
	
	return QT_OK;

fail:
	QT_ASSERTE(QT_FAILED(rv));
	StopListen(rv);
	return rv;
}

QtResult CQtAcceptorTcp::StopListen(QtResult aReason)
{
	if (m_Socket.GetHandle() != QT_INVALID_HANDLE) {
		m_pReactor->RemoveHandler(this);
		m_Socket.Close(aReason);
	}
	m_pSink = NULL;
	m_AcceptCount = 0;
	return QT_OK;
}

QT_HANDLE CQtAcceptorTcp::GetHandle() const 
{
	return m_Socket.GetHandle();
}

int CQtAcceptorTcp::OnInput(QT_HANDLE aFd)
{
	QT_ASSERTE(aFd == GetHandle());

	// create TCP transport with network reactor in the network thread.
	CQtTransportTcp *pTrans = new CQtTransportTcp(m_pReactorNetwork);
	if (!pTrans)
		return 0;

	int nSndBuffLen, nRcvBuffLen;
	int nOption;

	CQtInetAddr addrPeer;
	int nAddrLen = addrPeer.GetSize();
	QT_HANDLE sockNew = (QT_HANDLE)::accept(
		(QT_SOCKET)GetHandle(), 
		reinterpret_cast<struct sockaddr *>(const_cast<struct sockaddr_in *>(addrPeer.GetPtr())), 
#ifdef QT_WIN32
		&nAddrLen
#else 
#ifdef QT_MACOS
	#ifdef __i386__
		reinterpret_cast<socklen_t*>(&nAddrLen)
	#else
		&nAddrLen
	#endif		
#else
		reinterpret_cast<socklen_t*>(&nAddrLen)
#endif		
#endif // QT_WIN32
	);

	if (sockNew == QT_INVALID_HANDLE) {
#ifdef QT_WIN32
		errno = ::WSAGetLastError();
#endif //QT_WIN32
		QT_ERROR_TRACE_THIS("CQtAcceptorTcp::OnInput, accept() failed! err=" << errno);
		goto fail;
	}

	pTrans->GetPeer().SetHandle(sockNew);
	if (pTrans->GetPeer().Enable(CQtIPCBase::NON_BLOCK) == -1) {
		QT_ERROR_TRACE_THIS("CQtAcceptorTcp::OnInput, Enable(NON_BLOCK) failed! err=" << errno);
		goto fail;
	}

	nOption = pTrans->GetOption(QT_OPT_TRANSPORT_SND_BUF_LEN,  &nSndBuffLen);
	QT_ASSERTE(nOption == 0);
	nOption = pTrans->GetOption(QT_OPT_TRANSPORT_RCV_BUF_LEN,  &nRcvBuffLen);
	if(m_Interval && !(++m_AcceptCount % m_Interval))
	{
	QT_INFO_TRACE_THIS("CQtAcceptorTcp::OnInput,"
		" addr=" << addrPeer.GetIpDisplayName() <<
		" port=" << addrPeer.GetPort() << 
		" sockNew=" << sockNew << 
			" pTrans=" << pTrans <<
			" Send buffer len = " << nSndBuffLen <<
			" Receive buffer len = " << nRcvBuffLen <<
			" Accept Count = " << m_AcceptCount << 
			" sink = " << m_pSink);
	}

	QT_ASSERTE(m_pSink);
	if (m_pSink)
		m_pSink->OnConnectIndication(QT_OK, pTrans, this);
	return 0;

fail:
	delete pTrans;
	return 0;
}

int CQtAcceptorTcp::OnClose(QT_HANDLE aFd, MASK aMask)
{
	QT_ERROR_TRACE_THIS("CQtAcceptorTcp::OnClose, it's impossible!"
		" aFd=" << aFd <<
		" aMask=" << aMask);
	return 0;
}
#endif
