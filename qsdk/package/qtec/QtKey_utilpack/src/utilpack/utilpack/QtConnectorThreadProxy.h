/*------------------------------------------------------*/
/* Thread proxy wrapper for connector                   */
/*                                                      */
/* QtConnectorThreadProxy.h                             */
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

#ifndef QTCONNECORTHREADPROXY_H
#define QTCONNECORTHREADPROXY_H

#include "QtConnectionInterface.h"
#include "QtThreadManager.h"
#include "QtInetAddr.h"
#include "QtTimeValue.h"
#include "QtUtilClasses.h"
#include "QtAcceptorConnectorSinkThreadProxy.h"

class QT_OS_EXPORT CQtConnectorThreadProxy 
	: public IQtConnector
	, public CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>
	, public CQtReferenceControlMutilThread
	, public CQtStopFlag
{
public:
	CQtConnectorThreadProxy(
		CQtConnectionManager::CType aType,
		AQtThread *aThreadNetwork = NULL,
		AQtThread *aThreadUser = NULL);
	
	virtual ~CQtConnectorThreadProxy();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();
	virtual void OnReferenceDestory();

	// interface IQtAcceptorConnectorId
	virtual BOOL IsConnector();

	// interface IQtConnector
	virtual void AsycConnect(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrPeer,
		CQtTimeValue *aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL);

	virtual void CancelConnect(QtResult aReason);

	IQtAcceptorConnectorId* GetActualAcceptorConnectorId()
	{
		return m_pConActual.Get();
	}

	// we have to overlaod this function we want to ResetSink.
	void SetStopFlag();

private:
	AQtThread *m_pThreadUser;
	AQtThread *m_pThreadNetwork;
	CQtComAutoPtr<IQtConnector> m_pConActual;
	CQtConnectionManager::CType m_Type;

	friend class CEventAsycConnect;
	friend class CEventCancelConnect;
	friend class CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>;
	friend class CEventOnConnectIndication<CQtConnectorThreadProxy>;
};

	class CEventAsycConnect : public IQtEvent
	{
	public:
		CEventAsycConnect(
			CQtConnectorThreadProxy *aConnectorThreadProxy,
			IQtAcceptorConnectorSink *aSink, 
			const CQtInetAddr &aAddrPeer, 
			CQtTimeValue *aTimeout,
			CQtInetAddr *aAddrLocal);

		virtual ~CEventAsycConnect();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtConnectorThreadProxy> m_pOwnerThreadProxy;
		IQtAcceptorConnectorSink *m_pSink;
		CQtInetAddr m_addrPeer;
		CQtTimeValue m_tvTimeout;
		CQtTimeValue *m_pParaTimeout;
		CQtInetAddr *m_pParaAddrLocal;
		CQtInetAddr m_addrLocal;
	};

	class CEventCancelConnect : public IQtEvent
	{
	public:
		CEventCancelConnect(CQtConnectorThreadProxy *aConnectorThreadProxy, QtResult aReason);

		virtual ~CEventCancelConnect();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtConnectorThreadProxy> m_pOwnerThreadProxy;
        QtResult m_Reason;
	};

#endif // !QTCONNECORTHREADPROXY_H
