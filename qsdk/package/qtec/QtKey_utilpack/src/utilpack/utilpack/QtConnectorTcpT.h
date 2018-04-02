/*------------------------------------------------------*/
/* TCP socket for adapting Connector pattern            */
/*                                                      */
/* QtConnectorTcpT.h                                    */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

#ifndef QTCONNECTORTCPT_H
#define QTCONNECTORTCPT_H

#include "QtReactorInterface.h"
#include "QtSocket.h"
#include "QtObserver.h"
#include "QtInetAddr.h"
#include "QtDnsManager.h"

class CQtTimeValue;

template <class UpperType, class UpTrptType, class UpSockType>
class QT_OS_EXPORT CQtConnectorTcpT 
	: public AQtEventHandler
	, public AQtConnectorInternal
	, public IQtObserver 
{
public:
	CQtConnectorTcpT(IQtReactor *aReactor, UpperType &aUpper)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
		, m_pTransport(NULL)
		, m_bResolving(FALSE)
	{
	}

	virtual ~CQtConnectorTcpT()
	{
		Close();
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		QT_INFO_TRACE_THIS("CQtConnectorTcpT::Connect aAddr.Host = " << aAddr.GetHostName());
		int nRet = 0;
		const CQtInetAddr *pAddrConnect = &aAddr;
		if (aAddrLocal)
			m_addrLocal = *aAddrLocal;

#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		if (!aAddr.IsResolved()) {
			m_addrUnResolved = aAddr;
			pAddrConnect = &m_addrUnResolved;

			CQtComAutoPtr<CQtDnsRecord> pRecord;
			CQtString strHostName = m_addrUnResolved.GetHostName();
			QtResult rv = CQtDnsManager::Instance()->AsyncResolve(
				pRecord.ParaOut(),
				strHostName,
				this);
			if (QT_SUCCEEDED(rv)) {
				DWORD dwIp = *(pRecord->begin());
				rv = m_addrUnResolved.SetIpAddrBy4Bytes(dwIp);
				if (QT_FAILED(rv)) {
					QT_ERROR_TRACE_THIS("CQtConnectorTcpT::Connect, wrong ip addr from DSN,"
						" dwIp=" << dwIp << " hostname=" << strHostName);
					return -1;
				}
				if (m_bResolving) {
					CQtDnsManager::Instance()->CancelResolve(this);
					m_bResolving = FALSE;
				}
			}
			else if (rv == QT_ERROR_WOULD_BLOCK) {
				m_bResolving = TRUE;
				return 0;
			}
			else
				return -1;
		}
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		
		QT_ASSERTE_RETURN(!m_pTransport, -1);
		m_pTransport = new UpTrptType(m_pReactor);
		if (!m_pTransport) 
			return -1;

		nRet = Connect_i(m_pTransport, *pAddrConnect);
		if (nRet == 0) {
			// it rarely happens. we have to OnConnectIndication(QT_OK) to upper layer.
			QT_WARNING_TRACE_THIS("CQtConnectorTcpT::Connect, connect return 0.");
			nRet = m_pReactor->NotifyHandler(this, AQtEventHandler::WRITE_MASK);
		}
		else if (nRet == 1)
			nRet = 0;
		return nRet;
	}
	
	virtual int Close(QtResult aReason = QT_OK)
	{
		if (m_pTransport) {
			m_pReactor->RemoveHandler(this);
			delete m_pTransport;
			m_pTransport = NULL;
		}
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		if (m_bResolving) {
			CQtDnsManager::Instance()->CancelResolve(this);
			m_bResolving = FALSE;
		}
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		return 0;
	}

	/// interface AQtEventHandler
	virtual QT_HANDLE GetHandle() const 
	{
		QT_ASSERTE_RETURN(m_pTransport, QT_INVALID_HANDLE);
		return m_pTransport->GetHandle();
	}

	/// OnOutput() indicating Connect successful,
	/// OnClose() indicating Connect failed.
//	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);
	virtual int OnOutput(QT_HANDLE aFd = QT_INVALID_HANDLE)
	{
		QT_ASSERTE_RETURN(m_pTransport, QT_ERROR_FAILURE);
		QT_ASSERTE(aFd == m_pTransport->GetHandle());

#ifdef QT_SUPPORT_QOS
		CQtTransportBase::SetQos2Socket(m_pTransport->GetHandle());
#endif // QT_SUPPORT_QOS

		UpTrptType* pTrans = m_pTransport;
		m_pTransport = NULL;
		m_Upper.OnConnectIndication(QT_OK, pTrans, this);
		return 0;
	}

	virtual int OnClose(QT_HANDLE aFd, MASK aMask)
	{
		QT_ASSERTE(m_pTransport);
		if(m_pTransport)
		{
		QT_ASSERTE(aFd == m_pTransport->GetHandle());
		QT_ASSERTE(aMask == AQtEventHandler::CONNECT_MASK);
		}
		
		Close();
		m_Upper.OnConnectIndication(QT_ERROR_NETWORK_SOCKET_ERROR, NULL, this);
		return 0;
	}
	
	// interface IQtObserver
	virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL)
	{
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(CQtThreadManager::TT_NETWORK));
		QT_ASSERTE(!strcmp(aTopic, "DnsManager"));

		int nErr = *static_cast<int*>(aData);
		if (nErr || Connect(m_addrUnResolved, &m_addrLocal) == -1) {
			Close();
			m_Upper.OnConnectIndication(
				QT_ERROR_NETWORK_DNS_FAILURE,
				NULL, 
				this);
		}
	}

private:
	int Connect_i(UpTrptType *aTrpt, const CQtInetAddr &aAddr)
	{
		int nRet;
		UpSockType &sockPeer = aTrpt->GetPeer();
		QT_ASSERTE(sockPeer.GetHandle() == QT_INVALID_HANDLE);
		
		if (m_addrLocal == CQtInetAddr::s_InetAddrAny)
			nRet = sockPeer.Open(FALSE);
		else
			nRet = sockPeer.Open(FALSE, m_addrLocal);
		if (nRet == -1) {
			QT_ERROR_TRACE_THIS("CQtConnectorTcpT::Connect_i, Open() failed!"
				" laddr=" << m_addrLocal.GetIpDisplayName() <<
				" lport=" << m_addrLocal.GetPort() << 
				" err=" << errno);
			return -1;
		}
		if (sockPeer.Enable(CQtIPCBase::NON_BLOCK) == -1) {
			QT_ERROR_TRACE_THIS("CQtConnectorTcpT::Connect_i, Enable(NON_BLOCK) failed! err=" << errno);
			return -1;
		}

		QT_INFO_TRACE_THIS("CQtConnectorTcpT::Connect_i,"
			" addr=" << aAddr.GetIpDisplayName() << 
			" port=" << aAddr.GetPort() << 
			" laddr=" << m_addrLocal.GetIpDisplayName() <<
			" lport=" << m_addrLocal.GetPort() << 
			" fd=" << sockPeer.GetHandle());

		/// we regiester CONNECT_MASK prior to connect() to avoid lossing OnConnect()
		QtResult rv = m_pReactor->RegisterHandler(this, AQtEventHandler::CONNECT_MASK);
		if (QT_FAILED(rv))
			return -1;

		nRet = ::connect((QT_SOCKET)sockPeer.GetHandle(), 
						  reinterpret_cast<const struct sockaddr *>(aAddr.GetPtr()), 
						  aAddr.GetSize());
#ifdef QT_WIN32
		if (nRet == SOCKET_ERROR) {
			errno = ::WSAGetLastError();
			nRet = -1;
		}
#else // ! QT_WIN32
		if (nRet == -1 && errno == EINPROGRESS)
	#ifdef QT_MACOS
	#ifndef MachOSupport
			CFM_seterrno(EWOULDBLOCK);
    #else
			errno = EWOULDBLOCK;
	#endif	//MachOSupport		
    #else
			errno = EWOULDBLOCK;
	#endif
#endif // QT_WIN32

		if (nRet == -1) {
			if (errno == EWOULDBLOCK)
				return 1;
			else {
				QT_ERROR_TRACE_THIS("CQtConnectorTcpT::Connect_i, connect() failed!"
					" addr=" << aAddr.GetIpDisplayName() <<
					" port=" << aAddr.GetPort() <<
					"err="<< errno );
				return -1;
			}
		}
		else
			return 0;
	}

	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	UpTrptType *m_pTransport;
	CQtInetAddr m_addrUnResolved;
	CQtInetAddr m_addrLocal;
	BOOL m_bResolving;
};

#endif // !CONNECTORTCPT_H
