/*------------------------------------------------------*/
/* Package Sender classes                               */
/*                                                      */
/* CsPkgSender.h                                        */
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

//$Id: CsPkgSender.h,v 1.75 2010/01/28 00:08:32 cindy Exp $

#if !defined PKG_SENDER_H  && !defined (_NEW_PROTO_TP)
#define PKG_SENDER_H

#include "QtsBase.h"

class CPkgSender;
typedef CConnConnectorT<CPkgSender>		CPkgSenderConnector;

typedef ServerListT<CPkgSender>			CPkgSenderServerList;
typedef CConnAcceptorSinkT<CPkgSender>	CPkgSenderAcceptorSink;
typedef CConnAcceptorT<CPkgSender>		CPkgSenderAcceptor;

class QT_OS_EXPORT CPkgSender : public CConnBase
{
public:
	virtual QtResult SendData(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara = NULL);

	virtual QtResult SetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult Disconnect(
		QtResult aReason);
public:
	// interface IQtAcceptorConnectorSink
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId);

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

	virtual void OnTimer(
		CQtTimerWrapperID* aId);

public:
	//Unconnected time stamp
	CQtTimeValue GetDisconnTimestamp();
	void TryOnDisconnIndicate();
protected:
	void Reset();
	QtResult SendData_i();
public:
	CPkgSender();
	virtual ~CPkgSender();

	void NeedKeepAlive(BOOL bNeedKeepAlive);
	void SetConnConnector(CPkgSenderConnector* pConnConnector);
	//Cancel connect call, stop the handshake if connection hasn't made
	void CancelHandShake();
	
	//In order to keep one instance of Server List.
	void SetServerList(CPkgSenderServerList* pSvrList);
	void SetConnAcceptor(CPkgSenderAcceptor* pConnAcceptor);
protected:
	CQtMessageBlock *m_pmbLocSendData;	//local Data block for Send

	CPkgSenderConnector* m_pConnConnector;		//Client use

	CPkgSenderServerList* m_pSvrList;							//Server use
	CQtComAutoPtr<CPkgSenderAcceptor> m_pConnAcceptor;

	CQtTimeValue m_disconn_timestamp;
};

#endif	//PKG_SENDER_H
