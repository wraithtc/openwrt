
#include "QtBase.h"
#include "QtAcceptorUdp.h"
#include "QtTransportUdp.h"
#include "QtInetAddr.h"
#include "QtUtilTemplates.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

//Solaris use map, linux and mac use hashmap.
#ifndef QT_SOLARIS
#	include "QtHashMapT.h"
	
class CQtPairInetAddr
{
public:
	CQtPairInetAddr()
		: m_dwIpSrc(0)
		, m_dwIpDst(0)
		, m_wPortSrc(0)
		, m_wPortDst(0)
	{
	}

	CQtPairInetAddr(const CQtInetAddr &aSrc, const CQtInetAddr &aDst)
		: m_dwIpSrc(aSrc.GetPtr()->sin_addr.s_addr)
		, m_dwIpDst(aDst.GetPtr()->sin_addr.s_addr)
		, m_wPortSrc(aSrc.GetPtr()->sin_port)
		, m_wPortDst(aDst.GetPtr()->sin_port)
	{
	}

	DWORD GetHashValue() const 
	{
		// this hash function is copied from linux kernel
		// source code whose flie name is "net/ipv4/Tcp_ipv4.c".
		int h = ((m_dwIpSrc ^ m_wPortSrc) ^ (m_dwIpDst ^ m_wPortDst));
		h ^= h>>16;
		h ^= h>>8;
		return h;
	}

	bool operator == (const CQtPairInetAddr &aRight) const 
	{
		return m_dwIpSrc == aRight.m_dwIpSrc && 
			m_dwIpDst == aRight.m_dwIpDst && 
			m_wPortSrc == aRight.m_wPortSrc && 
			m_wPortDst == aRight.m_wPortDst;
	}

public:
	DWORD m_dwIpSrc;
	DWORD m_dwIpDst;
	WORD m_wPortSrc;
	WORD m_wPortDst;
};

typedef CQtHashMapT<CQtPairInetAddr, CQtComAutoPtr<CQtTransportUdp> > UdpTransportsType;

#else //!QT_SOLARIS
#	include <map>
	using namespace std;

class CQtPairInetAddr
{
public:
	CQtPairInetAddr()
		: m_dwIpSrc(0)
		, m_dwIpDst(0)
		, m_wPortSrc(0)
		, m_wPortDst(0)
	{
	}

	CQtPairInetAddr(const CQtInetAddr &aSrc, const CQtInetAddr &aDst)
		: m_dwIpSrc(aSrc.GetPtr()->sin_addr.s_addr)
		, m_dwIpDst(aDst.GetPtr()->sin_addr.s_addr)
		, m_wPortSrc(aSrc.GetPtr()->sin_port)
		, m_wPortDst(aDst.GetPtr()->sin_port)
	{
	}

public:
	DWORD m_dwIpSrc;
	DWORD m_dwIpDst;
	WORD m_wPortSrc;
	WORD m_wPortDst;
};

struct AddrlestCompare
{
	bool operator()(const CQtPairInetAddr& addr1, const CQtPairInetAddr& addr2)
	{
		if(addr1.m_dwIpSrc  > addr2.m_dwIpSrc)
			return TRUE;
		if(addr1.m_dwIpSrc == addr2.m_dwIpSrc && addr1.m_wPortSrc  > addr2.m_wPortSrc)
			return TRUE;
		if(addr1.m_dwIpSrc == addr2.m_dwIpSrc && addr1.m_wPortSrc  == addr2.m_wPortSrc && addr1.m_wPortDst > addr2.m_wPortDst)
			return TRUE;
		if(addr1.m_dwIpSrc == addr2.m_dwIpSrc && addr1.m_wPortSrc  == addr2.m_wPortSrc && addr1.m_wPortDst == addr2.m_wPortDst && addr1.m_dwIpDst > addr2.m_dwIpDst )
			return TRUE;
		return FALSE;
	}
};

typedef map<CQtPairInetAddr, CQtComAutoPtr<CQtTransportUdp>, AddrlestCompare> UdpTransportsType;
#endif // QT_SOLARIS

static UdpTransportsType *s_pUdpTransports;

// Don't make the hash_map static,
// because CQtComAutoPtr<CQtTransportUdp> may access thread info 
// which will be deleted in ~CQtThreadManager.
class CQtUdpTransportHashMap : public CQtCleanUpBase
{
public:
	CQtUdpTransportHashMap() 
#ifndef QT_SOLARIS
		: m_Transports(4096)
#endif
	{
		QT_ASSERTE(!s_pUdpTransports);
		s_pUdpTransports = &m_Transports;
	}

	virtual ~CQtUdpTransportHashMap()
	{
		QT_ASSERTE(s_pUdpTransports);
		s_pUdpTransports = NULL;
	}
	
public:
	UdpTransportsType m_Transports;
};


CQtAcceptorUdp::CQtAcceptorUdp()
{
	if (!s_pUdpTransports) {
		// it will be deleted when ~CQtCleanUpBase().
		new CQtUdpTransportHashMap();
//		CQtUdpTransportHashMap *pUthm = new CQtUdpTransportHashMap();
//		s_pUdpTransports = &(pUthm->m_Transports);
	}
	m_nRcvBuffLen = DEFAULT_RCVBUFF_SIZE * 4;
	m_nSndBuffLen = DEFAULT_SNDBUFF_SIZE * 4;
}

CQtAcceptorUdp::~CQtAcceptorUdp()
{
	StopListen(QT_OK);
}

QtResult CQtAcceptorUdp::
StartListen(IQtAcceptorConnectorSink *aSink, const CQtInetAddr &aAddrListen, int nTraceInterval)
{
	QtResult rv = QT_ERROR_NETWORK_SOCKET_ERROR;
	QT_ASSERTE_RETURN(m_Socket.GetHandle() == QT_INVALID_HANDLE, QT_ERROR_ALREADY_INITIALIZED);
	
	QT_ASSERTE(!m_pSink);
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
	m_pSink = aSink;

	m_Interval = nTraceInterval;
	m_AcceptCount = 0;
	DWORD dwRcv = m_nRcvBuffLen, dwSnd = m_nSndBuffLen;
	int nOption;
	int nRet = m_Socket.Open(aAddrListen);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtAcceptorUdp::StartListen, Open() failed!"
			" addr=" << aAddrListen.GetIpDisplayName() << 
			" port=" << aAddrListen.GetPort() <<
			" err=" << errno);
		rv = QT_ERROR_NETWORK_SOCKET_ERROR;
		goto fail;
	}

	nOption = m_Socket.SetOption(SOL_SOCKET, SO_SNDBUF,  &dwSnd, sizeof(DWORD));
	QT_ASSERTE(nOption == 0);
	nOption = m_Socket.SetOption(SOL_SOCKET, SO_RCVBUF,  &dwRcv, sizeof(DWORD));
	QT_ASSERTE(nOption == 0);

	rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::UDP_LINK_MASK);
	if (QT_FAILED(rv))
		goto fail;

	QT_INFO_TRACE_THIS("CQtAcceptorUdp::StartListen,"
		" addr=" << aAddrListen.GetIpDisplayName() <<
		" port=" << aAddrListen.GetPort() << 
		" aSink=" << aSink << 
		" fd=" << m_Socket.GetHandle() << 
		" trace interval = " << m_Interval);
	
	// aAddrListen may be "0.0.0.0".
	m_AddrLocol = aAddrListen;
	CQtStopFlag::SetStartFlag();
	return QT_OK;

fail:
	QT_ASSERTE(QT_FAILED(rv));
	StopListen(rv);
	return rv;
}

QtResult CQtAcceptorUdp::StopListen(QtResult )
{
	m_AcceptCount = 0;
	if (CQtStopFlag::IsFlagStopped())
		return QT_OK;

	// Don't clear because s_pUdpTransports is singleton.
	if (s_pUdpTransports) {
		UdpTransportsType::iterator iter = s_pUdpTransports->begin();
		while (iter != s_pUdpTransports->end()) {
			const CQtPairInetAddr &pairAddr = (*iter).first;
			DWORD dwIpDst = m_AddrLocol.GetPtr()->sin_addr.s_addr;
			WORD wPortDst = m_AddrLocol.GetPtr()->sin_port;
			if (pairAddr.m_dwIpDst == dwIpDst && pairAddr.m_wPortDst == wPortDst) {
				UdpTransportsType::iterator iterTmp = iter;
				++iterTmp;
				s_pUdpTransports->erase(iter);
				iter = iterTmp;
			}
			else {
				++iter;
			}
		}
	}
	
	if (m_Socket.GetHandle() != QT_INVALID_HANDLE) {
		m_pReactor->RemoveHandler(this);
		m_Socket.Close();
	}
	m_pSink = NULL;
	CQtStopFlag::SetStopFlag();
	return QT_OK;
}

QT_HANDLE CQtAcceptorUdp::GetHandle() const 
{
	return m_Socket.GetHandle();
}

int CQtAcceptorUdp::OnInput(QT_HANDLE )
{
	static char szBuf[CQtConnectionManager::UDP_SEND_MAX_LEN];
	CQtInetAddr addrRecv;
	int nRecv = m_Socket.RecvFrom(szBuf, sizeof(szBuf), addrRecv);
	if (nRecv <= 0) {
//		QT_WARNING_TRACE_THIS("CQtAcceptorUdp::OnInput, RecvFrom() failed!"
//			" nRecv=" << nRecv <<
//			" err=" << errno);
		return 0;
	}

	CQtTransportUdp *pTrans = NULL;
	CQtPairInetAddr addrPair(addrRecv, m_AddrLocol);
	UdpTransportsType::iterator iter = s_pUdpTransports->find(addrPair);
	if (iter == s_pUdpTransports->end()) {
		// create UDP transport with network reactor in the network thread.
		pTrans = new CQtTransportUdp(m_pReactorNetwork, addrRecv, this);
		if (!pTrans)
			return 0;

		pTrans->GetPeer().SetHandle(m_Socket.GetHandle());

		// Don't use operator[] because it does find() and insert().
		//m_Transports[addrPair] = pTrans;

		if(m_Interval && !(++m_AcceptCount % m_Interval))
		{
		QT_INFO_TRACE_THIS("CQtAcceptorUdp::OnInput,"
			" src_ip=" << addrRecv.GetIpDisplayName() << 
			" src_port=" << addrRecv.GetPort() << 
			" dst_ip=" << m_AddrLocol.GetIpDisplayName() << 
				" dst_port=" << m_AddrLocol.GetPort() << 
				" accepte count = " << m_AcceptCount);
		}

		// it will do AddRefenceControl() twice.
		UdpTransportsType::value_type nodeNew(addrPair, pTrans);
		s_pUdpTransports->insert(nodeNew);

		QT_ASSERTE(m_pSink);
		if (m_pSink)
			m_pSink->OnConnectIndication(QT_OK, pTrans, this);
	}
	else {
		pTrans = (*iter).second.Get();
	}

	QT_ASSERTE(pTrans);
	pTrans->OnReceiveCallback(szBuf, nRecv);
	return 0;
}

int CQtAcceptorUdp::OnClose(QT_HANDLE aFd, MASK aMask)
{
	QT_ERROR_TRACE_THIS("CQtAcceptorUdp::OnClose, it's impossible!"
		" aFd=" << aFd <<
		" aMask=" << aMask);
	QT_ASSERTE(FALSE);
	return 0;
}

QtResult CQtAcceptorUdp::
RemoveTransport(const CQtInetAddr &aAddr, CQtTransportUdp *aTrpt)
{
	// Needn't check because s_pUdpTransports is singleton.
//	if (s_pUdpTransports->empty()) {
	if (CQtStopFlag::IsFlagStopped()) {
		QT_WARNING_TRACE_THIS("CQtAcceptorUdp::RemoveTransport, the acceptor is stopped.");
		return QT_OK;
	}

//	QT_INFO_TRACE_THIS("CQtAcceptorUdp::RemoveTransport,"
//		" src_ip=" << aAddr.GetIpDisplayName() << 
//		" src_port=" << aAddr.GetPort() << 
//		" dst_ip=" << m_AddrLocol.GetIpDisplayName() << 
//		" dst_port=" << m_AddrLocol.GetPort());

	CQtPairInetAddr addrPair(aAddr, m_AddrLocol);
#ifdef QT_DEBUG
	UdpTransportsType::size_type nErase = 
#endif // QT_DEBUG
		s_pUdpTransports->erase(addrPair);
#ifdef QT_DEBUG
	QT_ASSERTE(nErase == 1);
#endif//! QT_DEBUG

	return QT_OK;
}
#endif
