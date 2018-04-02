/*------------------------------------------------------*/
/* SOCK4 and SOCK5 proxy connector                      */
/*                                                      */
/* QtConnectorSocksProxyT.h                             */
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

#ifndef QTCONNECTORSOCKSPROXYT_H
#define QTCONNECTORSOCKSPROXYT_H

#include "QtConnectorTcpT.h"
#include "QtTransportTcp.h"
#include "QtHttpUtilClasses.h"
#include "QtMessageBlock.h"
#include "QtHttpProxyManager.h"

template <class UpperType, class UpTrptType, class UpSockType>
class QT_OS_EXPORT CQtConnectorSocksProxyT 
	: public AQtConnectorInternal
	, public IQtTransportSink 
	, public IQtObserver
{
public:
	typedef CQtConnectorSocksProxyT SelfType;

	CQtConnectorSocksProxyT(
			IQtReactor *aReactor, 
			UpperType &aUpper, 
			CQtHttpProxyInfo* aProxyInfo = NULL)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
		, m_TcpConnector(aReactor, *this)
		, m_pTransport(NULL)
		, m_State(STATE_IDLE)
		, m_dwSvrIP(0)
		, m_wSvrPort(0)
	{
		m_bResolving = FALSE;
		m_ClientMethod = SOCKS_NO_AUTH;
		m_ProxyType = CQtHttpProxyInfo::SOCK4_PROXY;
		if (aProxyInfo)
			SetProxyInfo(aProxyInfo);
	}
	
	virtual ~CQtConnectorSocksProxyT()
	{
		if (CQtHttpProxyManager::Instance()) {
			CQtHttpProxyManager::Instance()->RemoveProxyAccess(this);
		}
		Close();
	}

	void SetProxyInfo(CQtHttpProxyInfo* aProxyInfo)
	{
		QT_ASSERTE_RETURN_VOID(aProxyInfo);

		m_pProxyInfo = aProxyInfo;
		m_ProxyType = m_pProxyInfo->GetProxyType();
		QT_ASSERTE(
			m_ProxyType == CQtHttpProxyInfo::SOCK4_PROXY || 
			m_ProxyType == CQtHttpProxyInfo::SOCK5_PROXY);
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::Connect,"
			" addr =" << aAddr.GetIpDisplayName() << " port = " << aAddr.GetPort() << " local = " << (aAddrLocal ? aAddrLocal->GetIpDisplayName() : " default"));
		QT_ASSERTE_RETURN(m_pProxyInfo, -1);
		//////////////////////////////////////////////////////////////////////////
		if ( CQtHttpProxyManager::Instance()) {
			CQtHttpProxyManager::Instance()->UpdateProxyProgress(m_pProxyInfo->GetHostName(), CQtHttpProxyManager::IS_CONNECTING, this);
		}
		//////////////////////////////////////////////////////////////////////////

		m_Addr = aAddr;
		if (!aAddr.IsResolved())  //try to resolve domain name first if the host is not a IP
		{
			
			CQtComAutoPtr<CQtDnsRecord> pRecord;
			CQtString strHostName = m_Addr.GetHostName();
			QtResult rv = CQtDnsManager::Instance()->AsyncResolve(
				pRecord.ParaOut(),
				strHostName,
				this);
			if (QT_SUCCEEDED(rv)) {
				DWORD dwIp = *(pRecord->begin());
				rv = m_Addr.SetIpAddrBy4Bytes(dwIp);
				if (QT_FAILED(rv)) {
					QT_ERROR_TRACE_THIS("CQtConnectorSocksProxyT::Connect, wrong ip addr from DSN,"
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
			}
			else
			{
				QT_ERROR_TRACE_THIS("CQtConnectorSocksProxyT::Connect, wrong ip addr from DSN,"
					<< " hostname=" << strHostName << " rv = " << rv);
				return -1;
			}
		}
		else
		{
			m_dwSvrIP = aAddr.GetPtr()->sin_addr.s_addr;
			m_wSvrPort = aAddr.GetPtr()->sin_port;
		}

		CQtInetAddr addrProxy(m_pProxyInfo->GetHostName().c_str(), m_pProxyInfo->GetPort());
		return m_TcpConnector.Connect(addrProxy);
	}

	virtual int Close(QtResult aReason = QT_OK)
	{
		m_TcpConnector.Close();
		if (m_pTransport) {
			m_pTransport = NULL;
		}
		m_pProxyInfo = NULL;
		m_State = STATE_IDLE;
		return 0;
	}

	int OnConnectIndication(
		QtResult aReason, 
		UpTrptType *aTrpt,
		AQtConnectorInternal *aId)
	{
//		QT_INFO_TRACE_THIS("OnConnectIndication");
		QT_ASSERTE(m_State == STATE_IDLE);
		QT_ASSERTE(&m_TcpConnector == aId);
		m_pTransport = aTrpt;
		//////////////////////////////////////////////////////////////////////////
		if ( m_pProxyInfo && CQtHttpProxyManager::Instance()) {
			CQtHttpProxyManager::Instance()->UpdateProxyProgress(m_pProxyInfo->GetHostName(), CQtHttpProxyManager::IS_CONNECTED, this);
		}
		//////////////////////////////////////////////////////////////////////////

		if (QT_SUCCEEDED(aReason)) {
			QT_ASSERTE(m_pTransport);
			aReason = m_pTransport->OpenWithSink(this);
			if (QT_SUCCEEDED(aReason))
			{
				if(m_bResolving)
				{
					QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::OnConnectIndication,"
					" resolve pending wait for it!");
					return QT_OK;
				}
				else
				{
					if(!m_Addr.IsResolved())
					{
						QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::OnConnectIndication,"
							" can not resolve it, try socks5!");
						m_ProxyType = CQtHttpProxyInfo::SOCK5_PROXY;
					}
					aReason = StartNewRequest();
				}
			}
		}

		if (QT_FAILED(aReason)) {
			Close();
			m_Upper.OnConnectIndication(aReason, NULL, this);
		}
		return 0;
	}

	QtResult StartNewRequest()
	{
		if(!m_pTransport)
		{
			QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest,"
				" transport has not built, waiting for it!");
			return QT_OK;
		}
		char lpBuf[256];
		DWORD dwBufLen = 0;
		if (m_State == STATE_IDLE) {
			if (m_ProxyType == CQtHttpProxyInfo::SOCK4_PROXY) {
				lpBuf[0] = 0x04; // SOCKS version 4
				lpBuf[1] = 0x01; // CD command code -- 1 for connect
				::memcpy(lpBuf + 2, &m_wSvrPort, sizeof(m_wSvrPort));
				::memcpy(lpBuf + 4, &m_dwSvrIP, sizeof(m_dwSvrIP));
				::memcpy(lpBuf + 8, "wbx", 3);
				lpBuf[11] = 0x00;
				dwBufLen = 12;
				m_State = STATE_CONNECT_REMOTE;
			}
			else {
				lpBuf[0] = 0x05;// SOCKS version 5
				lpBuf[1] = 0x01; // number of auth procotols we recognize auth protocols
				lpBuf[2] = 0x00; //no authentication required;
				// compliant implementations MUST implement GSSAPI
				// and SHOULD implement username/password and MAY
				// implement CHAP
				// TODO: we don't implement these
				//lpBuf[3] = 0x01; // GSSAPI
				//lpBuf[4] = 0x02; // username/password
				//lpBuf[5] = 0x03; // CHAP
				dwBufLen = lpBuf[1] + 2;
				m_State = STATE_CONNECT_PROXY;
			}
		}
		else if (m_State == STATE_CONNECT_PROXY) {
			QT_ASSERTE(m_ProxyType == CQtHttpProxyInfo::SOCK5_PROXY);
			lpBuf[0] = 0x05; // SOCKS version 5
			lpBuf[1] = 0x01; // CONNECT command
			lpBuf[2] = 0x00; // obligatory reserved field (perfect for MS tampering!)
			switch(m_ClientMethod) {
			case SOCKS_NO_AUTH:
				{
					if(!m_bResolving && m_Addr.IsResolved())
					{
						QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest IP = " <<
							m_Addr.GetIpDisplayName() << 
							" Port = " << m_Addr.GetPort());
						lpBuf[3] = SOCKS_IPV4; // encoding of destination address (1 == IPv4)
						memcpy(lpBuf + 4, &m_dwSvrIP,sizeof(m_dwSvrIP));
						memcpy(lpBuf + 8, &m_wSvrPort, sizeof(m_wSvrPort));
						dwBufLen = 10;
					}
					else if(!m_bResolving)//try with domain name
					{
						QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest domain = " <<
							m_Addr.GetHostName() << 
							" Port = " << m_Addr.GetPort());
						lpBuf[3] = SOCKS_DN;
						int nHostLen = strlen(m_Addr.GetHostName().c_str());
						QT_ASSERTE_RETURN(nHostLen < 248, QT_ERROR_FAILURE);
						lpBuf[4] = (BYTE)nHostLen;
						memcpy(lpBuf + 5, m_Addr.GetHostName().c_str(), nHostLen);
						memcpy(lpBuf + (5 + nHostLen), &m_wSvrPort, sizeof(m_wSvrPort));
						dwBufLen = 5 + sizeof(m_wSvrPort) + nHostLen;
					}
					else //in resolving, waiting...
					{
						QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest waiting for DNS resovling domain = " <<
							m_Addr.GetHostName() << 
							" Port = " << m_Addr.GetPort());
						return QT_OK;
					}
				}
				break;
//			case :
//				break;
			default:
				QT_ERROR_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest not support auth now!");
				return QT_ERROR_FAILURE;
			} 
			
			m_State = STATE_CONNECT_REMOTE;
		}
		else {
			QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest,"
				" wrong state=" << m_State);
			QT_ASSERTE(FALSE);
			return QT_ERROR_UNEXPECTED;
		}

		CQtMessageBlock mbSend(
			dwBufLen, 
			lpBuf, 
			CQtMessageBlock::DONT_DELETE, 
			dwBufLen);
		QtResult rv = m_pTransport->SendData(mbSend);
		if (QT_FAILED(rv)) {
			QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::StartNewRequest,"
				" SendData() failed! len=" << dwBufLen << 
				" rv=" << rv);
		}
		return rv;
	}

	virtual void OnReceive(
		CQtMessageBlock &aData,
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL)
	{
//		QT_INFO_TRACE_THIS("OnReceive");
		//////////////////////////////////////////////////////////////////////////
		if ( m_pProxyInfo && CQtHttpProxyManager::Instance()) {
			CQtHttpProxyManager::Instance()->UpdateProxyProgress(m_pProxyInfo->GetHostName(), CQtHttpProxyManager::IS_DONE, this);
		}
		//////////////////////////////////////////////////////////////////////////
		QT_ASSERTE(!aData.GetNext());
		LPCSTR lpBuf = aData.GetTopLevelReadPtr();
		DWORD dwBufLen = aData.GetTopLevelLength();
		if (m_State == STATE_CONNECT_PROXY) {
			QT_ASSERTE(m_ProxyType == CQtHttpProxyInfo::SOCK5_PROXY);
			if (dwBufLen >= 2 && (lpBuf[0] == 0x05 && 
				(lpBuf[1] == SOCKS_NO_AUTH || lpBuf[1] == SOCKS_NEED_AUTH))) {
				m_ClientMethod  = lpBuf[1];
				QtResult rv = StartNewRequest();
				if (QT_FAILED(rv))
					goto fail;
				return;
			}
			else {
				QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::OnReceive, fail1,"
					" len=" << dwBufLen <<
					" buf[0]=" << lpBuf[0] <<
					" buf[1]=" << lpBuf[1]);
				goto fail;
			}
		}
		else if (m_State == STATE_CONNECT_REMOTE) {
			if (m_ProxyType == CQtHttpProxyInfo::SOCK4_PROXY) {
				if (dwBufLen >= 8 && (lpBuf[0] == 0x00 && lpBuf[1] == 0x5A))
					m_State = STATE_SUCCESS;
				else if(dwBufLen >= 2 && lpBuf[0] == 0x05) //is socks5, try it
				{
					QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::OnReceive, not sock4, try sock5");
					m_State = STATE_IDLE;
					m_ProxyType = CQtHttpProxyInfo::SOCK5_PROXY;
					m_pTransport->Disconnect(0);
					m_pTransport = NULL;
					this->Connect(m_Addr);
					return;
				}
				else{
					QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::OnReceive, fail2,"
						" len=" << dwBufLen <<
						" buf[0]=" << lpBuf[0] <<
						" buf[1]=" << lpBuf[1]);
					goto fail;
				}
			}
			else {
				if (dwBufLen >= 10 && (lpBuf[0] == 0x05 && lpBuf[1] == 0x00))
					m_State = STATE_SUCCESS;
				else {
					QT_WARNING_TRACE_THIS("CQtConnectorSocksProxyT::OnReceive, fail3,"
						" len=" << dwBufLen <<
						" buf[0]=" << lpBuf[0] <<
						" buf[1]=" << lpBuf[1]);
					goto fail;
				}
			}
		}

		QT_ASSERTE(m_State == STATE_SUCCESS);
		m_Upper.OnConnectIndication(QT_OK, m_pTransport.ParaIn(), this);
		m_pTransport = NULL;
		return;

fail:
		Close();
		m_Upper.OnConnectIndication(QT_ERROR_NETWORK_SOCKET_ERROR, NULL, this);
		return;
	}

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL)
	{
		QT_ASSERTE(!"CQtConnectorSocksProxyT::OnSend, it shouldn't be called!");
	}

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId)
	{
//		QT_INFO_TRACE_THIS("OnDisconnect");
		Close();
		if (QT_SUCCEEDED(aReason))
			aReason = QT_ERROR_NETWORK_SOCKET_ERROR;
		m_Upper.OnConnectIndication(aReason, NULL, this);
	}

	// interface IQtObserver
	virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL)
	{
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(CQtThreadManager::TT_NETWORK));
		QT_ASSERTE(!strcmp(aTopic, "DnsManager"));
		
		int nErr = -1;
		if(aData)
			nErr = *static_cast<int*>(aData);
		if(nErr)
		{
			QT_ERROR_TRACE_THIS("CQtConnectorSocksProxyT::OnObserve errno = " << nErr);
		}
		if (!nErr && !m_Addr.IsResolved()) { //domain name resolve successful
			CQtComAutoPtr<CQtDnsRecord> pRecord;
			CQtString strHostName = m_Addr.GetHostName();
			QtResult rv = CQtDnsManager::Instance()->AsyncResolve(
				pRecord.ParaOut(),
				strHostName,
				this);
			if (QT_SUCCEEDED(rv)) {
				DWORD dwIP = *(pRecord->begin());
				rv = m_Addr.SetIpAddrBy4Bytes(dwIP);
				if (QT_FAILED(rv)) {
					QT_ERROR_TRACE_THIS("CQtConnectorSocksProxyT::OnObserve, wrong ip addr from DNS,"
						" IP=" << m_Addr.GetIpDisplayName() << " hostname=" << strHostName);
				}
				m_wSvrPort =  m_Addr.GetPtr()->sin_port;
				m_dwSvrIP = m_Addr.GetPtr()->sin_addr.s_addr;
				QT_INFO_TRACE_THIS("CQtConnectorSocksProxyT::OnObserve, addr from DNS,"<<
					" Ip = " << m_Addr.GetIpDisplayName() << 
					" m_wSvrPort=" << m_Addr.GetPort() << " hostname=" << strHostName);
				CQtDnsManager::Instance()->CancelResolve(this);
				m_bResolving = FALSE;
			}
		}
		StartNewRequest();
	}
	
private:

	enum SOCKS_STATE 
	{
		STATE_IDLE,
		STATE_CONNECT_PROXY,
		STATE_CONNECT_REMOTE,
		STATE_SUCCESS,
		STATE_FAILURE
	};
	
	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	CQtConnectorTcpT<SelfType, UpTrptType, UpSockType> m_TcpConnector;
	CQtComAutoPtr<UpTrptType> m_pTransport;
	SOCKS_STATE m_State;
	CQtComAutoPtr<CQtHttpProxyInfo> m_pProxyInfo;

	// follows are in network octet order.
	DWORD m_dwSvrIP;
	WORD  m_wSvrPort;

	//add by Victor 2007.6.11
	enum{
		SOCKS_IPV4	=	0x01,
		SOCKS_DN	=	0x03,
		SOCKS_IPV6	=	0x04
	};
	
	enum 
	{
		SOCKS_NO_AUTH	= 0x00,
		SOCKS_GSAPI,
		SOCKS_NEED_AUTH,
		SOCKS_IANA,
		SOCKS_PRIVATE	= 0x80,
		SOCKS_NO_SUPPORT	= 0xFF
	};
	CQtHttpProxyInfo::PROXY_TYPE m_ProxyType;
	CQtInetAddr m_Addr;
	BOOL	m_bResolving;
	BYTE	m_ClientMethod;

};

#endif // !QTCONNECTORSOCKSPROXYT_H
