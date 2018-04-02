/*------------------------------------------------------*/
/* Thread proxy class for IQtAcceptor                   */
/*                                                      */
/* QtAcceptorThreadProxy.h                              */
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

#ifndef QTACCEPTORTHREADPROXY_H
#define QTACCEPTORTHREADPROXY_H

#include "QtReferenceControl.h"
#include "QtConnectionInterface.h"
#include "QtAcceptorConnectorSinkThreadProxy.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

class QT_OS_EXPORT CQtAcceptorThreadProxy
	: public IQtAcceptor
	, public CQtReferenceControlMutilThread 
	, public CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>
	, public CQtStopFlag
{
public:
	CQtAcceptorThreadProxy(
		CQtConnectionManager::CType aType,
		AQtThread *aThreadNetwork = NULL,
		AQtThread *aThreadUser = NULL);
	
	virtual ~CQtAcceptorThreadProxy();
	
	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();
	virtual void OnReferenceDestory();

	// interface IQtAcceptorConnectorId
	virtual BOOL IsConnector();

	// interface IQtAcceptor
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrListen, 
		int nTraceInterval = 1);

	virtual QtResult StopListen(QtResult aReason);

	IQtAcceptorConnectorId* GetActualAcceptorConnectorId()
	{
		return m_pAcceptorActual.Get();
	}

	// we have to overlaod this function because class 
	// CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorBase> will invoke it.
	void SetStopFlag() { }

	QtResult SetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_INFO_TRACE_THIS("CQtAcceptorThreadProxy::SetOption aCommand = " << aCommand << " aArg = " << aArg);
		QT_ASSERTE_RETURN(aArg, QT_ERROR_FAILURE);
		switch(aCommand) {
		case QT_OPT_TRANSPORT_RCV_BUF_LEN:
			m_nRcvBuffLen = *(static_cast<int *>(aArg)) ;
			break;
		case QT_OPT_TRANSPORT_SND_BUF_LEN:
			m_nSndBuffLen = *(static_cast<int *>(aArg)) ;
			break;
		default:
			QT_ASSERTE_RETURN(0, QT_ERROR_FAILURE);
		}
		return QT_OK;
	}
	
	QtResult GetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_INFO_TRACE_THIS("CQtAcceptorThreadProxy::GetOption aCommand = " << aCommand << " aArg = " << aArg);
		switch(aCommand) {
		case QT_OPT_TRANSPORT_RCV_BUF_LEN:
			*(static_cast<int *>(aArg)) = m_nRcvBuffLen;
			break;
		case QT_OPT_TRANSPORT_SND_BUF_LEN:
			*(static_cast<int *>(aArg)) = m_nSndBuffLen;
			break;
		default:
			QT_ASSERTE_RETURN(0, QT_ERROR_FAILURE);
		}
		return QT_OK;
	}
	

private:
	AQtThread *m_pThreadUser;
	AQtThread *m_pThreadNetwork;
	CQtComAutoPtr<IQtAcceptor> m_pAcceptorActual;
	CQtConnectionManager::CType m_Type;

	int m_nRcvBuffLen;
	int m_nSndBuffLen;

	friend class CEventStartListen;
	friend class CEventStopListen;

	friend class CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>;
	friend class CEventOnConnectIndication<CQtAcceptorThreadProxy>;
};

	class CEventStartListen : public IQtEvent
	{
	public:
		CEventStartListen(
			CQtAcceptorThreadProxy *aThreadProxy,
			IQtAcceptorConnectorSink *aSink,
			const CQtInetAddr &aAddrListen, 
			int nTraceInterval);

		virtual ~CEventStartListen();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtAcceptorThreadProxy> m_pOwnerThreadProxy;
		IQtAcceptorConnectorSink *m_pSink;
		CQtInetAddr m_addrListen;
		int m_nTraceInterval;
	};

	class CEventStopListen : public IQtEvent
	{
	public:
		CEventStopListen(
			CQtAcceptorThreadProxy *aConnectorThreadProxy,
			QtResult aReason);

		virtual ~CEventStopListen();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtAcceptorThreadProxy> m_pOwnerThreadProxy;
		QtResult m_Reason;
	};

#endif
#endif // !QTACCEPTORTHREADPROXY_H
