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

#if !defined PKG_SENDER_H  && !defined (_NEW_PROTO_TP)
#define PKG_SENDER_H

#include "QtsBase.h"

class CPkgSender;
typedef CConnConnectorT<CPkgSender> CPkgSenderConnector;

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

protected:
	void Reset();
	QtResult SendData_i();
public:
	CPkgSender();
	virtual ~CPkgSender();

	void NeedKeepAlive(BOOL bNeedKeepAlive);
	void SetConnConnector(CPkgSenderConnector* pConnConnector);
protected:
	CQtMessageBlock *m_pmbLocData;	//local Data block for Send

	CQtComAutoPtr<CPkgSenderConnector> m_pConnConnector;		//Only Client use
};

#endif	//PKG_SENDER_H
