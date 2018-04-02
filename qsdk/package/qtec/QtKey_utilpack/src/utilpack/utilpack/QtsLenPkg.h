/*------------------------------------------------------*/
/* PDU Length Package classes                           */
/*                                                      */
/* CsLenPkg.h                                           */
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

#include "QtsPkgSender.h"


#if !defined LEN_PKG_H  && !defined (_NEW_PROTO_TP)
#define LEN_PKG_H

class CLenPkgConn;
typedef CConnConnectorT<CLenPkgConn>	CLenPkgConnConnector;

typedef ServerListT<CLenPkgConn>		CLenPkgConnServerList;
typedef CConnAcceptorSinkT<CLenPkgConn>	CLenPkgConnAcceptorSink;
typedef CConnAcceptorT<CLenPkgConn>		CLenPkgConnAcceptor;

class QT_OS_EXPORT CLenPkgConn : public CPkgSender
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
	CLenPkgConn();
	virtual ~CLenPkgConn();

	void NeedKeepAlive(BOOL bNeedKeepAlive);
	void SetConnConnector(CLenPkgConnConnector* pConnConnector);
	//Cancel connect call, stop the handshake if connection hasn't made
	void CancelHandShake();
	
	//In order to keep one instance of Server List.
	void SetServerList(CLenPkgConnServerList* pSvrList);
	void SetConnAcceptor(CLenPkgConnAcceptor* pConnAcceptor);
protected:
	void Reset4Recv();
protected:
	CQtMessageBlock *m_pmbSendLocData;	//local Data block for Send
	CQtMessageBlock *m_pmbRecvLocData;	//local Data block for Recv
	CQtMessageBlock *m_pmbLastGet;		//Last get Data block for OnRecv

	DWORD m_dwPDULen;	//Data Length should be received
	DWORD m_dwDataLen;

	CLenPkgConnConnector* m_pConnConnector;		//Client use

	CLenPkgConnServerList* m_pSvrList;							//Server use
	CQtComAutoPtr<CLenPkgConnAcceptor> m_pConnAcceptor;

	CQtTimeValue m_disconn_timestamp;
};

#endif	//LEN_PKG_H
