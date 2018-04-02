/*------------------------------------------------------*/
/* Thread proxy wrapper for transport                   */
/*                                                      */
/* QtTransportThreadProxy.h                             */
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

#ifndef QTTRANSPORTTHREADPROXY_H
#define QTTRANSPORTTHREADPROXY_H

#include "QtConnectionInterface.h"
#include "QtThreadManager.h"
#include "QtInetAddr.h"
#include "QtTimeValue.h"
#include "QtUtilClasses.h"
#include "QtTimerWrapperID.h"
#include <list>

class QT_OS_EXPORT CQtTransportThreadProxy 
	: public IQtTransport
	, public IQtTransportSink
	, public CQtReferenceControlMutilThread
	, public CQtStopFlag
	, public CQtTimerWrapperIDSink
{
public:
	CQtTransportThreadProxy(
		IQtTransport *aActual, 
		AQtThread *aThreadNetwork,
		AQtThread *aThreadUser,
		CQtConnectionManager::CType aType);

	virtual ~CQtTransportThreadProxy();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();
	virtual void OnReferenceDestory();

	// interface IQtTransport
	virtual QtResult OpenWithSink(IQtTransportSink *aSink);
	virtual IQtTransportSink* GetSink();
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL);
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult Disconnect(QtResult aReason);

	// interface IQtTransportSink
	virtual void OnReceive(
		CQtMessageBlock &aData,
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL);

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL);

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId);

	virtual void OnTimer(CQtTimerWrapperID* aId);

private:
	QtResult Send_i(
		CQtMessageBlock *aData, 
		CQtTransportParameter* aPara,
		BOOL aIsDuplicated);

	CQtComAutoPtr<IQtTransport> m_pTransportActual;
	IQtTransportSink *m_pSinkActual;
	AQtThread *m_pThreadUser;
	AQtThread *m_pThreadNetwork;
	CQtConnectionManager::CType m_Type;
	CQtTimerWrapperID m_TimerReference0;

	struct CItem 
	{
		CItem(CQtMessageBlock *aMb, CQtTransportParameter *aPara);
		~CItem();

		CQtMessageBlock *m_pMbSend;
		CQtTransportParameter m_TransportParameter;
		CQtTransportParameter *m_pParaTransportParameter;
	};
	typedef std::list<CItem> SendBufferType;

	// for send buffer, we don't need mutex becase.
	// <m_SendBuffer> and <m_bIsBufferFull> are all modified in the network thread.
	// <m_bNeedOnSend> is modified in the user thread.
	BOOL m_bIsBufferFull;
	BOOL m_bNeedOnSend;
	SendBufferType m_SendBuffer;

	friend class CEventSendData;
	friend class CEventDisconnect;
	friend class CEventOnReceive;
	friend class CEventOnSend;
	friend class CEventOnDisconnect;
};

	class CEventSendData : public IQtEvent
	{
	public:
		CEventSendData(
			CQtTransportThreadProxy *aThreadProxy,
			CQtMessageBlock &aData, 
			CQtTransportParameter *aPara);

		virtual ~CEventSendData();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtTransportThreadProxy> m_pOwnerThreadProxy;
		CQtMessageBlock *m_pMessageBlock;
		CQtTransportParameter m_TransportParameter;
		CQtTransportParameter *m_pParaTransportParameter;
	};
	
	class CEventDisconnect : public IQtEvent
	{
	public:
		CEventDisconnect(
			CQtTransportThreadProxy *aThreadProxy,
			QtResult aReason);

		virtual ~CEventDisconnect();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtTransportThreadProxy> m_pOwnerThreadProxy;
		QtResult m_Reason;
	};
	
	class CEventOnReceive : public IQtEvent
	{
	public:
		CEventOnReceive(
			CQtTransportThreadProxy *aThreadProxy,
			CQtMessageBlock &aData,
			IQtTransport *aTrptId,
			CQtTransportParameter *aPara);

		virtual ~CEventOnReceive();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtTransportThreadProxy> m_pOwnerThreadProxy;
		CQtMessageBlock *m_pData;
		IQtTransport *m_pTrptId;
		CQtTransportParameter m_TransportParameter;
		CQtTransportParameter *m_pParaTransportParameter;
	};

	class CEventOnSend : public IQtEvent
	{
	public:
		CEventOnSend(
			CQtTransportThreadProxy *aThreadProxy,
			IQtTransport *aTrptId,
			CQtTransportParameter *aPara);

		virtual ~CEventOnSend();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtTransportThreadProxy> m_pOwnerThreadProxy;
		IQtTransport *m_pTrptId;
		CQtTransportParameter m_TransportParameter;
		CQtTransportParameter *m_pParaTransportParameter;
	};

	class CEventOnDisconnect : public IQtEvent
	{
	public:
		CEventOnDisconnect(
			CQtTransportThreadProxy *aThreadProxy,
			QtResult aReason,
			IQtTransport *aTrptId);

		virtual ~CEventOnDisconnect();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtTransportThreadProxy> m_pOwnerThreadProxy;
		QtResult m_Reason;
		IQtTransport *m_pTrptId;
	};

#endif // !QTTRANSPORTTHREADPROXY_H
