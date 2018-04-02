
#include "QtBase.h"
#include "QtConnectorWrapper.h"
#include "QtTimeValue.h"
#include "QtTransportBase.h"
#include "QtConnectorTcpT.h"
#include "QtConnectorUdpT.h"
#include "QtTransportTcp.h"
#include "QtTransportUdp.h"
#if defined QT_WIN32 || defined QT_PORT_CLIENT
#include "QtConnectorProxyT.h"
#endif
#ifdef QT_SUPPORT_OPENSSL
#include "QtConnectorOpenSslT.h"
#endif // QT_SUPPORT_OPENSSL

CQtConnectorWrapper::CQtConnectorWrapper()
	: m_pReactor(NULL)
	, m_pSink(NULL)
	, m_pConnector(NULL)
	, m_Type(CQtConnectionManager::CTYPE_NONE)
	, m_bClosed(TRUE)
{
}

CQtConnectorWrapper::~CQtConnectorWrapper()
{
	Close_i();
	delete m_pConnector;
}

DWORD CQtConnectorWrapper::AddReference()
{
	return CQtReferenceControlSingleThread::AddReference();
}

DWORD CQtConnectorWrapper::ReleaseReference()
{
	return CQtReferenceControlSingleThread::ReleaseReference();
}

QtResult CQtConnectorWrapper::Init(CQtConnectionManager::CType aType)
{
	QT_ASSERTE_RETURN(!m_pConnector, QT_ERROR_ALREADY_INITIALIZED);
	m_Type = aType;

	QT_ASSERTE(!m_pReactor);
	m_pReactor = CQtThreadManager::Instance()->GetThreadReactor(CQtThreadManager::TT_NETWORK);
	QT_ASSERTE_RETURN(m_pReactor, QT_ERROR_NOT_INITIALIZED);

	switch(aType) {
	case CQtConnectionManager::CTYPE_TCP:
	case CQtConnectionManager::CTYPE_WEBEX_GATEWAY_TCP_DIRECT:
	case CQtConnectionManager::CTYPE_WEBEX_GATEWAY_TCP_WITH_BROWSE_PROXY:
		
		m_pConnector = new 
			CQtConnectorTcpT<CQtConnectorWrapper, CQtTransportTcp, CQtSocketTcp>
			(m_pReactor, *this);
		break;

	case CQtConnectionManager::CTYPE_UDP:
		m_pConnector = new
			CQtConnectorUdpT<CQtConnectorWrapper, CQtTransportUdp, CQtSocketUdp>
			(m_pReactor, *this);
		break;

#ifdef QT_SUPPORT_OPENSSL
	case CQtConnectionManager::CTYPE_SSL_DIRECT:
	case CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY:
	case CQtConnectionManager::CTYPE_SSL:
	case CQtConnectionManager::CTYPE_WEBEX_GATEWAY_SSL_WITH_BROWER_PROXY:
	case CQtConnectionManager::CTYPE_WEBEX_GATEWAY_SSL_DIRECT:
		m_pConnector = new 
			CQtConnectorOpenSslT<CQtConnectorWrapper>(m_pReactor, *this, aType);
		break;
#endif // QT_SUPPORT_OPENSSL

#if defined QT_WIN32 || defined QT_PORT_CLIENT
	case CQtConnectionManager::CTYPE_TCP_WITH_BROWER_PROXY:
		m_pConnector = new 
			 CQtConnectorProxyT<CQtConnectorWrapper, CQtTransportTcp, CQtSocketTcp>(m_pReactor, *this, FALSE);
		break;
		
	case CQtConnectionManager::CTYPE_HTTP_WITH_BROWER_PROXY: //add for HTTP proxy, 5/21 2009  Victor
		m_pConnector = new 
			CQtConnectorProxyT<CQtConnectorWrapper, CQtTransportTcp, CQtSocketTcp>(m_pReactor, *this, TRUE);
		break;
#endif
	default:
		QT_WARNING_TRACE_THIS("CQtConnectorWrapper::Init, error type=" << aType);
		Close_i();
		return QT_ERROR_INVALID_ARG;
	}

	if (!m_pConnector) {
		Close_i();
		return QT_ERROR_OUT_OF_MEMORY;
	}
	else {
		return QT_OK;
	}
}

void CQtConnectorWrapper::
AsycConnect(IQtAcceptorConnectorSink* aSink, const CQtInetAddr& aAddrPeer, 
			CQtTimeValue* aTimeout, CQtInetAddr *aAddrLocal)
{
	QT_ASSERTE(m_pConnector);

	QT_ASSERTE(!m_pSink);
	m_pSink = aSink;
	QT_ASSERTE(m_pSink);

	QT_ASSERTE(m_bClosed);
	m_bClosed = FALSE;

	//if type is webex gateway, then save the peer addr
	if(m_Type & CQtConnectionManager::CTYPE_WEBEX_GATEWAY)
		m_AddrPeer = aAddrPeer;

	QT_INFO_TRACE_THIS("CQtConnectorWrapper::AsycConnect,"
		" addr=" << aAddrPeer.GetIpDisplayName() <<
		" port=" << aAddrPeer.GetPort() << 
		" m_Type=" << m_Type << 
		" m_pConnector=" << m_pConnector);

	int nRet = -1;
	if (m_pConnector && m_pSink)
		nRet = m_pConnector->Connect(aAddrPeer, aAddrLocal);
	if (nRet == -1) {
		QT_WARNING_TRACE_THIS("CQtConnectorWrapper::AsycConnect, connect failed."
			" addr=" << aAddrPeer.GetIpDisplayName() <<
			" err=" << errno);
		m_pReactor->ScheduleTimer(
			this, 
			reinterpret_cast<LPVOID>(QT_ERROR_NETWORK_CONNECT_ERROR), 
			CQtTimeValue(0, 0), 
			1);
		return;
	}
	else if(nRet == QT_ERROR_NETWORK_NO_PROXY)
	{
		QT_WARNING_TRACE_THIS("CQtConnectorWrapper::AsycConnect, connect failed. that have no proxy "
			" addr=" << aAddrPeer.GetIpDisplayName() <<
			" err=" << errno);
		m_pReactor->ScheduleTimer(
			this, 
			reinterpret_cast<LPVOID>(QT_ERROR_NETWORK_NO_PROXY), 
			CQtTimeValue(0, 0), 
			1);
		return;
	}

	if (aTimeout) {
		m_pReactor->ScheduleTimer(
			this, 
			reinterpret_cast<LPVOID>(QT_ERROR_NETWORK_CONNECT_TIMEOUT), 
			*aTimeout, 
			1);
	}
}

int CQtConnectorWrapper::
OnConnectIndication(QtResult aReason, IQtTransport *aTrpt, AQtConnectorInternal *aId)
{
	QT_ASSERTE(m_pConnector);
	QT_ASSERTE(m_pSink);
	QT_ASSERTE(aId == m_pConnector);
	if(aTrpt)
	{
		CQtInetAddr addrLocal1;
		aTrpt->GetOption(QT_OPT_TRANSPORT_LOCAL_ADDR, &addrLocal1);
		QT_INFO_TRACE_THIS("CQtConnectorWrapper::OnConnectIndication,"
			" aReason=" << aReason <<
			" aTrpt=" << aTrpt <<
			" aId=" << aId << 
			" local ip = " << addrLocal1.GetIpDisplayName() << 
			" port = " << addrLocal1.GetPort());
	}
	else
	{
		QT_INFO_TRACE_THIS("CQtConnectorWrapper::OnConnectIndication,"
		" aReason=" << aReason <<
		" aTrpt=" << aTrpt <<
		" aId=" << aId);
	}
	
	
		
	
	CQtComAutoPtr<IQtTransport> pTransport(aTrpt);
	if (QT_FAILED(aReason)) {
		Close_i();
		m_pSink->OnConnectIndication(aReason, NULL, this);
	}
	else {
		Close_i();
		//send user data for WEBEX_GATEWAY
		if(m_Type & CQtConnectionManager::CTYPE_WEBEX_GATEWAY)
		{
			int length = m_AddrPeer.GetUserData().length();
			if(length == 0){
				QT_WARNING_TRACE_THIS("CQtConnectorWrapper::OnConnectIndication, No user data");
				m_pSink->OnConnectIndication(QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE,NULL,this);
				return 0;
			}else{
				if(aTrpt)
				{
				char *buf = new char[length];
				memcpy(buf,m_AddrPeer.GetUserData().c_str(),length);
				CQtMessageBlock mb(length,buf,CQtMessageBlock::DONT_DELETE,length);
				aTrpt->SendData(mb);
				delete []buf;
			}
		}
		}
		m_pSink->OnConnectIndication(aReason, aTrpt, this);
	}
	return 0;
}

void CQtConnectorWrapper::CancelConnect(QtResult aReason)
{
	Close_i(aReason);
	m_pSink = NULL;
}

void CQtConnectorWrapper::Close_i(QtResult aReason)
{
	if (m_bClosed)
		return;
	m_bClosed = TRUE;
	
	// Don't cleanup resources due to connect again.
	if (m_pConnector) {
		m_pConnector->Close(aReason);
//		delete m_pConnector;
//		m_pConnector = NULL;
	}
	if (m_pReactor) {
		m_pReactor->CancelTimer(this);
//		m_pReactor = NULL;
	}

	// can't empty m_pSink because callback follows Close_i()
//	m_pSink = NULL;
}

void CQtConnectorWrapper::OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg)
{
	QT_ASSERTE(m_pSink);
	
	QtResult rvReason = reinterpret_cast<QtResult>(aArg);
	QT_INFO_TRACE_THIS("CQtConnectorWrapper::OnTimeout, connect failed. rvReason = " << rvReason << " sink = " << m_pSink << ", type = " << m_Type);
	switch (rvReason)
	{
	case QT_ERROR_NETWORK_CONNECT_ERROR:
		break;
	case QT_ERROR_NETWORK_CONNECT_TIMEOUT:
//		QT_INFO_TRACE_THIS("CQtConnectorWrapper::OnTimeout, connect timeout.");
		break;
	case QT_ERROR_NETWORK_NO_PROXY:		
//		QT_INFO_TRACE_THIS("CQtConnectorWrapper::OnTimeout, no proxy.");
		break;
	default:
//		QT_WARNING_TRACE_THIS("CQtConnectorWrapper::OnTimeout, unkown nReason=" << rvReason);
		QT_ASSERTE(FALSE);
		return;
	}

	Close_i(rvReason);
	m_pSink->OnConnectIndication(rvReason, NULL, this);
}

BOOL CQtConnectorWrapper::IsConnector()
{
	return TRUE;
}
