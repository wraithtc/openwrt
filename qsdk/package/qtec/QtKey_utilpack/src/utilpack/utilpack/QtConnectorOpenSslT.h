/*------------------------------------------------------*/
/* SSL connector using OpenSSL library                  */
/*                                                      */
/* ConnectorOpenSslT.h                                  */
/*                                                      */
/* Copyright (C) QTEC Inc.              */
/* All rights reserved                                  */
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

#ifndef CONNECTOROPENSSL_H
#define CONNECTOROPENSSL_H

#include "QtReactorInterface.h"
#include "QtSocket.h"
#include "QtTransportTcp.h"
#include "QtTransportOpenSsl.h"
#include "QtConnectionInterface.h"
#include "QtConnectorProxyT.h"
//#include "QtTimerWrapperID.h"

#ifndef QT_MMP
#include <openssl/ssl.h>
#include <openssl/err.h>
#else
#include "cmssl.h"
#endif

template <class UpperType>
class CQtConnectorOpenSslT 
	: public AQtEventHandler
	, public AQtConnectorInternal
//	, public CQtTimerWrapperIDSink  //for proxy priority
{
public:
	typedef CQtConnectorOpenSslT SelfType;
	typedef CQtSocketTcp SockType;
	typedef CQtTransportOpenSsl TrptType;
	
	CQtConnectorOpenSslT(
			IQtReactor *aReactor, 
			UpperType &aUpper, 
			CQtConnectionManager::CType aType)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
		, m_ConnectorTcp(aReactor, *this)
		, m_ConnectorProxy(aReactor, *this, TRUE)
		, m_Type(aType)
		, m_TypeOrigin(aType)
//		, m_bConnSucceed(FALSE)
	{
	}

	virtual ~CQtConnectorOpenSslT()
	{
		Close();
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		CQtInetAddr addrSsl(aAddr);
		if(!addrSsl.IsLock())
		addrSsl.SetPort(443);
		m_Type = m_TypeOrigin;
		
/*		if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)) {
			m_ConnectTimer.Schedule(this, (LONG)30, 1);
		int nRet = m_ConnectorProxy.Connect(addrSsl, aAddrLocal);
			if (nRet == -1) {
				m_ConnectorProxy.Close();
				QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
			}
		}
*/ //for proxy higher priority

#ifdef QT_MMP
		// budingc 02/06/2006, disable SSL direct if detecting browser proxy.
//		if (CQtHttpProxyManager::Instance()->IsBrowerProxySet()) {
//			QT_WARNING_TRACE_THIS("CQtConnectorOpenSslT::Connect, disable SSL direct.");
//			QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT);
//			if (QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY))
//				return -1;
//		}
		// budingc 04/28/2006, connecting with ssl proxy first, try ssl direct if proxy failed in 15s.
		if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT) && 
			QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)) 
		{
			QT_ERROR_TRACE_THIS("CQtConnectorOpenSslT::Connect, MMP session should"
				" invoke proxy or direct individally!");
		}
#endif // QT_MMP

		if (QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT) &&
			QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)) 
		{
			QT_ERROR_TRACE_THIS("CQtConnectorOpenSslT::Connect, wrong type=" << m_Type);
			return -1;
		}
		int nRet = 0;		
		if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT)) {
			nRet = m_ConnectorTcp.Connect(addrSsl, aAddrLocal);
			if (nRet == -1) {
				m_ConnectorTcp.Close();
				QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT);
			}
		}
		if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)) {
			nRet = m_ConnectorProxy.Connect(addrSsl, aAddrLocal);
			if (nRet == -1 || nRet == QT_ERROR_NETWORK_NO_PROXY) {
				m_ConnectorProxy.Close();
				QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
				return nRet;
			}
		}

		return nRet == -1 ? -1 : 0;
	}

	virtual int Close(QtResult aReason = QT_OK)
	{
		m_ConnectorTcp.Close();
		m_ConnectorProxy.Close();
		if (m_pTransport) {
			// Needn't RemoveHandler here because Transport will do it.
		//	m_pReactor->RemoveHandler(this);
			m_pTransport->Disconnect(QT_OK);
			m_pTransport = NULL;
		}
		return 0;
	}
	
	int OnConnectIndication(
		QtResult aReason, 
		TrptType *aTrpt,
		AQtConnectorInternal *aId)
	{
		int nRet;
		QtResult rv;
		SSL *pSsl;
		QT_INFO_TRACE_THIS("CQtConnectorOpenSslT::OnConnectIndication,"
			" aReason=" << aReason << 
			" aTrpt=" << aTrpt << 
			" aId=" << aId);
		
		if (QT_FAILED(aReason)) {
			QT_ASSERTE(!aTrpt);

			if (&m_ConnectorTcp == aId) {
				QT_ASSERTE(QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT));
				QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT);
				m_ConnectorTcp.Close();
			}
			else if (&m_ConnectorProxy == aId) {
				QT_ASSERTE(QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY));
				QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
				m_ConnectorProxy.Close();
			}

			if (QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT) &&
				QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)) 
			{
				goto fail;
			}

			// wait for another connector.
			return 0;
		}

		if (&m_ConnectorTcp == aId) {
			QT_ASSERTE(QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT));
//			if(!QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY))
//			{
			QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
			m_ConnectorProxy.Close();
//				m_bConnSucceed = TRUE;
//			}
		}
		else if (&m_ConnectorProxy == aId) {
//			QT_INFO_TRACE_THIS("CQtConnectorOpenSslT::OnConnectIndication,"
//				" proxy connection succeed!");
			QT_ASSERTE(QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY));
			QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_DIRECT);
			m_ConnectorTcp.Close();
/*			m_bConnSucceed = TRUE;
			if(m_pTransport) //directly connection already built?
				m_pTransport->Disconnect(QT_OK);
			m_ConnectTimer.Cancel();
*/
		}

		m_pTransport = aTrpt;
//		if(m_bConnSucceed) ///connect succeed, if proxy enable and not back, wait it in timer
//		{
		QT_ASSERTE(m_pTransport);

		if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY)
			rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK);
		else 
			rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
		if (QT_FAILED(rv) && rv != QT_ERROR_FOUND) {
			QT_ERROR_TRACE_THIS("CQtConnectorOpenSslT::OnConnectIndication,"
				" RegisterHandler(READ_MASK|WRITE_MASK) failed!");
			goto fail;
		}

		nRet = m_pTransport->InitSsl(m_Type);
		if (nRet == -1)
			goto fail;

		pSsl = m_pTransport->GetSslPtr();
		QT_ASSERTE(pSsl);
#ifdef QT_MMP
		CQTSSL::SSL_set_connect_state(pSsl);
#else
		::SSL_set_connect_state(pSsl);
#endif
		nRet = DoHandshake();
		if (nRet == -1)
			goto fail;
		else if (nRet == 0) {
			QT_WARNING_TRACE_THIS("CQtConnectorOpenSslT::OnConnectIndication,"
				" connnect return 0.");
		}
//		}
		return 0;

fail:
		Close();
		if (QT_SUCCEEDED(aReason))
			aReason = QT_ERROR_NETWORK_SOCKET_ERROR;
		m_Upper.OnConnectIndication(aReason, NULL, this);
		return 0;
	}
	//add by Victor Cui 2/10 2006, for proxy
/*	virtual void OnTimer(CQtTimerWrapperID* aId)
	{
		QT_INFO_TRACE_THIS("CQtConnectorOpenSslT::OnTimer proxy connect failed and use directly connection");
		QT_ASSERTE(aId == &m_ConnectTimer);
		int nRet;
		QtResult rv;
		if(m_pTransport)
		{
			QT_ASSERTE(m_pTransport);
			QT_CLR_BITS(m_Type, CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
			m_ConnectorProxy.Close();

		if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY)
			rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK);
		else 
			rv = m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
			if (QT_FAILED(rv) && rv != QT_ERROR_FOUND) {
				QT_ERROR_TRACE_THIS("CQtConnectorOpenSslT::OnTimer,"
					" RegisterHandler(READ_MASK|WRITE_MASK) failed!");
				goto fail;
			}

			nRet = m_pTransport->InitSsl(m_Type);
			if (nRet == -1)
				goto fail;

			SSL *pSsl = m_pTransport->GetSslPtr();
			QT_ASSERTE(pSsl);
	#ifdef QT_MMP
			CQTSSL::SSL_set_connect_state(pSsl);
	#else
			::SSL_set_connect_state(pSsl);
	#endif
			nRet = DoHandshake();
			if (nRet == -1)
				goto fail;
			else if (nRet == 0) {
				QT_WARNING_TRACE_THIS("CQtConnectorOpenSslT::OnTimer,"
					" connnect return 0.");
			}
			return;
		}
		QT_INFO_TRACE_THIS("CQtConnectorOpenSslT::OnTimer connect no response");
fail:
		Close();
		m_Upper.OnConnectIndication(QT_ERROR_NETWORK_SOCKET_ERROR, NULL, this);
	}
*/

	virtual QT_HANDLE GetHandle() const 
	{
		QT_ASSERTE_RETURN(m_pTransport, QT_INVALID_HANDLE);
		return m_pTransport->GetHandle();
	}
	
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE)
	{
		QT_ASSERTE_RETURN(m_pTransport, -1);
		int nRet = m_pTransport->RecvFromSocket();
		if (nRet <= 0)
			return nRet;
		
		nRet = DoHandshake();
		if (nRet == -1)
			return nRet;
		else
			return 0;
	}

	virtual int OnOutput(QT_HANDLE aFd = QT_INVALID_HANDLE)
	{
		int nRet = DoHandshake();
		if (nRet == -1)
			return nRet;
		else
			return 0;
	}

	virtual int OnClose(QT_HANDLE aFd, MASK aMask)
	{
		Close();
		m_Upper.OnConnectIndication(QT_ERROR_NETWORK_SOCKET_ERROR, NULL, this);
		return 0;
	}

private:
	int DoHandshake()
	{
		QT_ASSERTE_RETURN(m_pTransport, -1);

		SSL *pSsl = m_pTransport->GetSslPtr();
#ifdef QT_MMP
		int nConn = CQTSSL::SSL_connect(pSsl);
		int nErr = CQTSSL::SSL_get_error(pSsl, nConn);
#else
		int nConn = ::SSL_connect(pSsl);
		int nErr = ::SSL_get_error(pSsl, nConn);
#endif
		switch (nErr) {
		case SSL_ERROR_NONE: {
			CQtComAutoPtr<TrptType> pTrans = m_pTransport;
			m_pTransport = NULL;
			m_Upper.OnConnectIndication(QT_OK, pTrans.ParaIn(), this);
			return 0;
							 }

		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			return 1;

		default:
			CQtTransportOpenSsl::TraceOpenSslError(
				"CQtConnectorOpenSslT::DoHandshake, SSL_connect() failed!", 
				this);
			return -1;
		}
	}

	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	CQtConnectorTcpT<SelfType, TrptType, SockType> m_ConnectorTcp;
	CQtConnectorProxyT<SelfType, TrptType, SockType> m_ConnectorProxy;
	CQtComAutoPtr<TrptType> m_pTransport;
	CQtConnectionManager::CType m_Type;
	CQtConnectionManager::CType m_TypeOrigin;
//	CQtTimerWrapperID m_ConnectTimer;			//Add by victor 2006/2/10, for proxy higher priority
//	BOOL m_bConnSucceed;
};

#endif // !CONNECTOROPENSSL_H
