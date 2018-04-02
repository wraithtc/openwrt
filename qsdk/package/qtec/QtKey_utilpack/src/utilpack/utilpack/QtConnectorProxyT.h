/*------------------------------------------------------*/
/* Proxy (Http and Socks) connector                     */
/*                                                      */
/* QtConnectorProxyT.h                                  */
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

#ifndef QTCONNECTORPROXYT_H
#define QTCONNECTORPROXYT_H

#include "QtConnectorTcpT.h"
#include "QtConnectorSocksProxyT.h"
#include "QtConnectorHttpProxyT.h"
#include "QtInetAddr.h"
#include "QtTimerWrapperID.h"
#include "QtHttpProxyManager.h"
#include "QtObserver.h"
#ifdef QT_WIN32
#include "HttpAuthInfoGetterFromSaved.h"
#include "HttpAuthInfoGetterFromEureka_ER3_TP.h"
#endif

template <class UpperType, class UpTrptType, class UpSockType>
class QT_OS_EXPORT CQtConnectorProxyT 
	: public AQtConnectorInternal
	, public IQtObserver
	, public CQtTimerWrapperIDSink
{
public:
	typedef CQtConnectorProxyT SelfType;
	typedef CQtTransportTcp TrptType;
	typedef CQtSocketTcp SockType;

	enum{
		TIME_SEC = 10,
		TIME_USEC = 0
	};
	CQtConnectorProxyT(IQtReactor *aReactor, UpperType &aUpper, BOOL aNeedTunnel)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
		, m_TcpConnect(aReactor, *this)
		, m_HttpConnetor(aReactor, *this)
		, m_SocksConnector(aReactor, *this)
		, m_bNeedTunnel(aNeedTunnel)
		, m_bOnConnectFlag(FALSE)
	{
		QT_INFO_TRACE_THIS("CQtConnectorProxyT::CQtConnectorProxyT()");
		/// to start the proxy finding.
		//can not be here, Victor, 2006.8.14, crash bug, if proxy info like https://......pac will be crash
		m_pHttpProxyManager = NULL;//CQtHttpProxyManager::Instance();
		//		QT_ASSERTE(m_pHttpProxyManager);
	}

	virtual ~CQtConnectorProxyT()
	{
		QT_INFO_TRACE_THIS("CQtConnectorProxyT::~CQtConnectorProxyT()");
		Close();
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		QT_INFO_TRACE_THIS("CQtConnectorProxyT::Connect," << " Host = " << aAddr.GetHostName() << 
			" addr =" << aAddr.GetIpDisplayName() << " port = " << aAddr.GetPort() << " local = " << (aAddrLocal ? aAddrLocal->GetIpDisplayName() : " default"));
		if(!m_pHttpProxyManager)
			m_pHttpProxyManager = CQtHttpProxyManager::Instance();
		
		m_bOnConnectFlag = FALSE; //reset the flag when try to connect
		
		m_Timer.Cancel();
		m_Timer.Schedule(this, CQtTimeValue(TIME_SEC, TIME_USEC), 1);

		QT_ASSERTE_RETURN(m_pHttpProxyManager, QT_ERROR_FAILURE);
		m_AddrSvr = aAddr;
		QT_ASSERTE(!aAddrLocal);

		CQtString aHost = aAddr.GetHostName().empty() ? m_AddrSvr.GetIpDisplayName() : m_AddrSvr.GetHostName();
		for ( ; ; ) {
			IQtHttpProxyInfoGetter::NextReturn nrGet;
			nrGet = m_pHttpProxyManager->GetProxyInfo(
//				m_AddrSvr.GetIpDisplayName(),
				aHost,
				m_AddrSvr.GetPort(),
				m_pProxyInfo.ParaOut());
			if (nrGet == IQtHttpProxyInfoGetter::NEXT_SUCCESS) {
				// budingc 05/04/2006, handle "DIRECT" or bypass.
//				if (!m_pProxyInfo) {
					// we don't support "DIRECT" dut to it's proxy connector.
//					continue;
//				}
                if (!m_pProxyInfo || m_pProxyInfo->GetProxyType() == CQtHttpProxyInfo::DIRECT) {
					if (m_TcpConnect.Connect(m_AddrSvr) == 0)
						return 0;
					m_TcpConnect.Close();
				}
				else if (!m_bNeedTunnel) {
					CQtInetAddr addrTcp(
						m_pProxyInfo->GetHostName().c_str(), 
						m_pProxyInfo->GetPort());
					if (m_TcpConnect.Connect(addrTcp) == 0)
						return 0;
					m_TcpConnect.Close();
				}
				else if (m_pProxyInfo->GetProxyType() <= CQtHttpProxyInfo::HTTPS_PROXY) {
					// for HTTP proxy
					m_HttpConnetor.SetProxyInfo(m_pProxyInfo.ParaIn());
					if (m_HttpConnetor.Connect(m_AddrSvr, aAddrLocal) == 0)
						return 0;
					m_HttpConnetor.Close();
				}
				else {
					/// for SOCKS proxy
					m_SocksConnector.SetProxyInfo(m_pProxyInfo.ParaIn());
					if (m_SocksConnector.Connect(m_AddrSvr, aAddrLocal) == 0)
						return 0;
					m_SocksConnector.Close();
				}
			}
			else if (nrGet == IQtHttpProxyInfoGetter::NEXT_WOULDBLCOK) {
				m_pHttpProxyManager->AddObserver(this);
				return 0;
			}
			else {
				QT_WARNING_TRACE_THIS("CQtConnectorProxyT::Connect,"
					" GetProxyInfo() none."
					" addr=" << m_AddrSvr.GetIpDisplayName() << 
					" port=" << m_AddrSvr.GetPort());
				return QT_ERROR_NETWORK_NO_PROXY;
			}
		}

		QT_ASSERTE(!"CQtConnectorProxyT::Connect, can't reach here!");
		return -1;
	}
	
	//10/20 2006, if the cached proxy is same as now proxy, clear it
	BOOL IsSameAsCachedProxy()
	{
		if(!m_pHttpProxyManager)
			m_pHttpProxyManager = CQtHttpProxyManager::Instance();
		
		if(m_pHttpProxyManager && m_pProxyInfo)
		{
			CQtComAutoPtr<CQtHttpProxyInfo> pCachedProxy;
			IQtHttpProxyInfoGetter::NextReturn nrGet;
			nrGet = m_pHttpProxyManager->GetProxyInfo(
				m_AddrSvr.GetIpDisplayName(),
				m_AddrSvr.GetPort(),
				pCachedProxy.ParaOut());
			if (nrGet == IQtHttpProxyInfoGetter::NEXT_SUCCESS) 
			{
				if(m_pProxyInfo)
					QT_INFO_TRACE_THIS("CQtConnectorProxyT::IsSameAsCachedProxy,"
					"  current used proxy =" << m_pProxyInfo->GetHostName() << " cached in manager proxy = " << pCachedProxy->GetHostName());
				if( pCachedProxy->GetHostName().length() == m_pProxyInfo->GetHostName().length()
					&& strncasecmp(pCachedProxy->GetHostName().c_str(), m_pProxyInfo->GetHostName().c_str(), pCachedProxy->GetHostName().length()) == 0
					&& pCachedProxy->GetPort() == m_pProxyInfo->GetPort() 
					&& pCachedProxy->GetProxyType() == m_pProxyInfo->GetProxyType())
					return TRUE;
			}
		}
		return FALSE;
	}

	virtual void OnTimer(CQtTimerWrapperID* aId)
	{
		if(IsAuthDialogPopup())
		{
			QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnTimer dialog popup, do nothing");
			return;
		}
		else
		{
			if(m_pProxyInfo && CQtHttpProxyManager::Instance()->GetProxyProgress(m_pProxyInfo->GetHostName(), this) == CQtHttpProxyManager::IS_CONNECTING)
				{
					QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnTimer failed to connect proxy, try next if it is available");
					OnConnectIndication(QT_ERROR_NETWORK_CONNECT_TIMEOUT, NULL, this);
				}
			else
			{
				QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnTimer other status, do nothing");
			}
		}
	}

	virtual int Close(QtResult aReason = QT_OK)
	{
		m_Timer.Cancel();
		QT_INFO_TRACE_THIS("CQtConnectorProxyT::Close, aReason = " << aReason );
		////remove the proxy from proxy list if
			//1, the connect requirement has no get response
			//2, the reason should be timeout
			//3, if proxy item tried this time is same as the latest item in the list
		if(!m_bOnConnectFlag && QT_ERROR_NETWORK_CONNECT_TIMEOUT == aReason && IsSameAsCachedProxy()) 
		{
			QT_INFO_TRACE_THIS("CQtConnectorProxyT::Close  same proxy as before, clear cached and try next if it is available");
			m_pHttpProxyManager->ClearCacheProxy(
				m_AddrSvr.GetIpDisplayName(), 
				m_AddrSvr.GetPort());
		}
		m_TcpConnect.Close();
		m_HttpConnetor.Close();
		m_SocksConnector.Close();
		if(m_pHttpProxyManager)
			m_pHttpProxyManager->RemoveObserver(this);
		return 0;
	}
	
	// we have use IQtTransport instead TrptType due to ConnectorHttpProxy
	int OnConnectIndication(
		QtResult aReason, 
		IQtTransport *aTrpt,
		AQtConnectorInternal *aId)
	{
		QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication, aReason = " << aReason <<
			" Transport = " << aTrpt << " AQtConnectorInternal = " << aId);
		QtResult rv;

		m_Timer.Cancel();
		m_bOnConnectFlag = TRUE; //the connect requirement already got result
		CQtComAutoPtr<IQtTransport> pAutoDelete(aTrpt);
		if (QT_SUCCEEDED(aReason)) {
			QT_ASSERTE(aTrpt);
			BOOL bAlive = FALSE;
			aTrpt->GetOption(QT_OPT_TRANSPORT_SOCK_ALIVE, &bAlive);
			if (!bAlive
#ifdef QT_LINUX
				&& errno != 111			//that ubuntu 9.04 has not allow doing this operator
#endif
				) 
			{
				QT_ERROR_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication,"
					" transport isn't alive! err=" << errno);
				goto fail;
			}

			QT_HANDLE hdNew = QT_INVALID_HANDLE;
			aTrpt->GetOption(QT_OPT_TRANSPORT_FD, &hdNew);
			QT_ASSERTE(hdNew != QT_INVALID_HANDLE);

			QT_HANDLE hdNULL = QT_INVALID_HANDLE;
			rv = aTrpt->SetOption(QT_OPT_TRANSPORT_FD, &hdNULL);
			QT_ASSERTE(QT_SUCCEEDED(rv));

			UpTrptType *pTrans = new UpTrptType(m_pReactor);
			pTrans->GetPeer().SetHandle(hdNew);
			m_Upper.OnConnectIndication(QT_OK, pTrans, this);
			return 0;
		}

fail:
		Close();
		if (QT_SUCCEEDED(aReason))
			aReason = QT_ERROR_NETWORK_SOCKET_ERROR;
		else if(QT_ERROR_PROXY_RETRYTIMES_OVER == aReason || QT_ERROR_PROXY_CACNEL_BY_USER == aReason)
		{
			QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication  aReason = " << aReason);
		}
		else if (aReason != QT_ERROR_NETWORK_PROXY_SERVER_UNAVAILABLE && m_pProxyInfo) {
			//10/20 2006, if already move by others, not clear and try cached 				
			if(IsSameAsCachedProxy())  //already retry this proxy and failed, get and try next
			{
				QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication  same proxy as before, clear cached and try next if it is available");
				rv = m_pHttpProxyManager->ClearCacheProxy(
					m_AddrSvr.GetIpDisplayName(), 
					m_AddrSvr.GetPort());
			}
			else //not same, the proxy already move by others, try it 
			{
				QT_INFO_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication  not same proxy as before, not clear cached");
				rv = QT_ERROR_FOUND;
			}
			if (rv == QT_ERROR_FOUND) {
				int  nRet = Connect(m_AddrSvr);
				if (-1 == nRet)
					aReason = QT_ERROR_NETWORK_UNKNOWN_ERROR;
				else if(QT_ERROR_NETWORK_NO_PROXY == nRet) //have no proxy, will failed to connect
				{
					QT_WARNING_TRACE_THIS("CQtConnectorProxyT::OnConnectIndication have no proxy");
					aReason = QT_ERROR_NETWORK_NO_PROXY;
				}
				else //waiting for connecting response
					return 0;
			}
		}
		m_Upper.OnConnectIndication(aReason, NULL, this);
		return 0;
	}

	virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL)
	{
		QT_ASSERTE(!strcmp(aTopic, "HttpProxyManager"));
		
		//Victor Cui, 9/28 2006 if user cancel it, terminate immediately
		QtResult aResult = QT_OK;
		if(aData)
			aResult =  *reinterpret_cast<QtResult *>(aData);
		if(aResult == QT_ERROR_PROXY_CACNEL_BY_USER || aResult == QT_ERROR_PROXY_RETRYTIMES_OVER)
		{
			Close();
			m_pProxyInfo = NULL;
			m_Upper.OnConnectIndication(
				aResult,
				NULL, 
				this);
		}
		else 
		{
			int nRet = Connect(m_AddrSvr);
			if(-1 == nRet) {
				QT_WARNING_TRACE_THIS("CQtConnectorProxyT::OnObserve, failed to connect");
				Close();
				m_pProxyInfo = NULL;
				m_Upper.OnConnectIndication(QT_ERROR_NETWORK_UNKNOWN_ERROR,	NULL, this);
			}
			else if(QT_ERROR_NETWORK_NO_PROXY == nRet)
			{
				QT_WARNING_TRACE_THIS("CQtConnectorProxyT::OnObserve have no proxy, failed to connect");
				Close();
				m_pProxyInfo = NULL;
				m_Upper.OnConnectIndication(nRet, NULL, this);
			}
		}
	}
			
private:
	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	CQtConnectorTcpT<SelfType, TrptType, SockType> m_TcpConnect;
	CQtConnectorHttpProxyT<SelfType, TrptType, SockType> m_HttpConnetor;
	CQtConnectorSocksProxyT<SelfType, TrptType, SockType> m_SocksConnector;
	CQtComAutoPtr<CQtHttpProxyInfo> m_pProxyInfo;
	CQtInetAddr m_AddrSvr;
	CQtHttpProxyManager *m_pHttpProxyManager;
	BOOL m_bNeedTunnel;
	BOOL m_bOnConnectFlag;
	CQtTimerWrapperID m_Timer;					//Timer
};

#endif // !QTCONNECTORPROXYT_H
