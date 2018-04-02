/*------------------------------------------------------*/
/* Http proxy connector                                 */
/*                                                      */
/* QtConnectorHttpProxyT.h                              */
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

#ifndef QTCONNECTORHTTPPROXYT_H
#define QTCONNECTORHTTPPROXYT_H

#include "QtReactorInterface.h"
#include "QtChannelHttpClient.h"
#include "QtHttpUrl.h"

template <class UpperType, class UpTrptType, class UpSockType>
class QT_OS_EXPORT CQtConnectorHttpProxyT 
	: public AQtConnectorInternal
	, public IQtChannelSink
{
public:
	CQtConnectorHttpProxyT(IQtReactor *aReactor, UpperType &aUpper)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
	{
	}
	
	virtual ~CQtConnectorHttpProxyT()
	{
		Close();
	}

	void SetProxyInfo(CQtHttpProxyInfo* aProxyInfo)
	{
		QT_ASSERTE_RETURN_VOID(aProxyInfo);

		m_pProxyInfo = aProxyInfo;
		QT_ASSERTE(
			m_pProxyInfo->GetProxyType() == CQtHttpProxyInfo::HTTP_PROXY || 
			m_pProxyInfo->GetProxyType() == CQtHttpProxyInfo::HTTPS_PROXY);
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		//we should use host name to connect when that using proxy 
		QT_INFO_TRACE_THIS("CQtConnectorHttpProxyT::Connect, Host = " << aAddr.GetHostName() <<
			" addr =" << aAddr.GetIpDisplayName() << " port = " << aAddr.GetPort() << " local = " << (aAddrLocal ? aAddrLocal->GetIpDisplayName() : " default"));
		QT_ASSERTE_RETURN(m_pProxyInfo, -1);
		QT_ASSERTE(!m_HttpChannel);
		m_AddrServer = aAddr;
		
		CQtString aHost = aAddr.GetHostName().empty() ? aAddr.GetIpDisplayName() : aAddr.GetHostName();
		CQtString strURL;
		strURL.reserve(128);
		if (aAddr.GetPort() == 443) {
			strURL = "https://";
//			strURL += aAddr.GetIpDisplayName();
			strURL += aHost;
		}
		else {
			strURL = "http://";
//			strURL += aAddr.GetIpDisplayName();
			strURL += aHost;
			char szPort[32];
			snprintf(szPort, sizeof(szPort), ":%d", static_cast<int>(aAddr.GetPort()));
			strURL += szPort;
		}
		CQtComAutoPtr<CQtHttpUrl> pURL;
		QtResult rv = CQtChannelManager::Instance()->CreateUrl(
			pURL.ParaOut(),
			strURL);
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtConnectorHttpProxyT::Connect,"
				" unknown URL=" << strURL);
			return -1;
		}

		// don't call CQtChannelManager->CreateChannelHttpClient() 
		// in order to get CQtChannelHttpClient* directly.
		m_HttpChannel = new CQtChannelHttpClient(pURL.ParaIn(), m_pProxyInfo.ParaIn());
		QT_ASSERTE_RETURN(m_HttpChannel, -1);

		// SetRequestMethod CONNECT before calling AsyncOpen().
		rv = m_HttpChannel->SetRequestMethod("Connect");
		QT_ASSERTE(QT_SUCCEEDED(rv));

		// budingc 05/02/2006, skip content length.
		BOOL bSkipContentLength = TRUE;
		rv = m_HttpChannel->SetOption(
			QT_OPT_CHANNEL_HTTP_PARSER_SKIP_CONTENT_LENGTH, &bSkipContentLength);
		QT_ASSERTE(QT_SUCCEEDED(rv));

		rv = m_HttpChannel->AsyncOpen(this);
		return QT_SUCCEEDED(rv) ? 0 : -1;
	}
	
	virtual int Close(QtResult aReason = QT_OK)
	{
		if (m_HttpChannel) {
			m_HttpChannel->Disconnect(QT_OK);
			m_HttpChannel = NULL;
		}
		m_pProxyInfo = NULL;
		return 0;
	}

	virtual void OnConnect(QtResult aReason, IQtChannel *aChannelId)
	{
		QT_ASSERTE(m_HttpChannel.Get() == aChannelId);
		if (QT_SUCCEEDED(aReason)) {
			CQtMessageBlock mbZero(0UL);
			aReason = m_HttpChannel->SendData(mbZero);
			QT_ASSERTE(QT_SUCCEEDED(aReason));
			if (QT_SUCCEEDED(aReason))
				return;
		}

		QT_ASSERTE(QT_FAILED(aReason));
		OnDisconnect(aReason, aChannelId);
	}

	virtual void OnReceive(
		CQtMessageBlock &aData, 
		IQtTransport *aTrptId, 
		CQtTransportParameter *aPara)
	{
		QT_ASSERTE(m_HttpChannel.Get() == aTrptId);

		LONG lState = 0;
		QtResult rv = m_HttpChannel->GetResponseStatus(lState);
		QT_ASSERTE(QT_SUCCEEDED(rv));

		QT_INFO_TRACE_THIS("CQtConnectorHttpProxyT::OnReceive,"
			" len=" << aData.GetChainedLength() <<
			" lState=" << lState);

		if (lState == 200) {
			CQtComAutoPtr<IQtTransport> pTrans;
			m_HttpChannel->TransferTransport(pTrans.ParaOut());
			rv = m_HttpChannel->Disconnect(QT_OK);
			m_HttpChannel = NULL;
			QT_ASSERTE(pTrans);
			m_Upper.OnConnectIndication(QT_OK, pTrans.ParaIn(), this);
		}
		else {
			if(lState == 401 || lState == 407)
			{
				QT_WARNING_TRACE_THIS("CQtConnectorHttpProxyT::OnReceive, do nothing");
				return;
			}
			QtResult rvReason = QT_ERROR_FAILURE;
			if (lState >= 500 && lState < 600)
				rvReason = QT_ERROR_NETWORK_PROXY_SERVER_UNAVAILABLE;
			OnDisconnect(rvReason, aTrptId);
		}
	}

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL)
	{
		QT_ASSERTE(!"CQtConnectorHttpProxyT::OnSend, it shouldn't be called!");
	}

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId)
	{
		Close();
		if (QT_SUCCEEDED(aReason))
			aReason = QT_ERROR_NETWORK_SOCKET_ERROR;
		m_Upper.OnConnectIndication(aReason, NULL, this);
	}

private:
	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	CQtInetAddr m_AddrServer;
	CQtComAutoPtr<CQtChannelHttpClient> m_HttpChannel;
	CQtComAutoPtr<CQtHttpProxyInfo> m_pProxyInfo;
};

#endif // !QTCONNECTORHTTPPROXYT_H
