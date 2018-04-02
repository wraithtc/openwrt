/*------------------------------------------------------*/
/* Connection with Package classes                      */
/*                                                      */
/* CsPkg.h                                              */
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

//$Id: CsPkg.h,v 1.77 2010/01/28 00:08:32 cindy Exp $

#if !defined PKG_H  && !defined (_NEW_PROTO_TP)
#define PKG_H

#include "QtsBase.h"
#include "QtsRlbTcp.h"

#define SEND_BUF_LEN_MAX4PKG		65536	//64K bytes 

#define SHORT_MSEC_INTERVAL1		(long)200000	//200ms
#define SHORT_MSEC_INTERVAL2		(long)500000	//500ms
#define SHORT_MSEC_INTERVAL3		(long)1000000	//1s

#define MAX_CONN_RESP_NO_RECV		6		//UDP case, retry for 3 times sending ConnReq

#define TV_FOR_RELEASE				(long)60		//1minutes

class CPkgConnClient;
class CPkgConnServer;

typedef ServerListT<CPkgConnServer>			CPkgConnServerList;

typedef CConnAcceptorSinkT<CPkgConnServer>	CPkgConnAcceptorSink;
typedef CConnAcceptorT<CPkgConnServer>		CPkgConnAcceptor;

typedef CConnConnectorT<CPkgConnClient>		CPkgConnConnector;

class QT_OS_EXPORT CPkgConn : public CCsConn
{
public:
	virtual QtResult SetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult Disconnect(QtResult aReason);
	
	virtual QtResult SendData(
						  CQtMessageBlock &aData, 
						  CQtTransportParameter *aPara);
	virtual QtResult SendDisconn(QtResult aReason);
public:
	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId) = 0;

	virtual void OnTimer(
		CQtTimerWrapperID* aId) = 0;

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL);
protected:
	//Internal functions
	virtual QtResult SendKeepAlive();
	virtual QtResult SendDataFromSendBuf();
	virtual void ACK2PeerIfPossiable();//Amount number of PDUs have been received, we may do ACK back to peer
	QtResult SendData_i();
	void Reset();
public:
	CPkgConn(DWORD dwMaxSendBufLen);
	virtual ~CPkgConn();
protected:
	BOOL m_bPkgNeedBuf;

	//local Data block for Send, when PKG-TCP with no SendBuf
	CQtMessageBlock *m_pmbLocSendData;	

	int m_nUngetDataCnt;//Counter for times haven't received data

	BOOL m_bConnIndicate2Upper;// If has Connect Indicate to Upper layer already
	
	DWORD m_dwServerUnavailTimeout; //allow uplayer to set the timeout
};

class QT_OS_EXPORT CPkgConnClient : public CPkgConn
{
public:
	// interface IQtAcceptorConnectorSink
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId);

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId);
	
	virtual void OnTimer(CQtTimerWrapperID* aId);

public:
	void SetConnConnector(CPkgConnConnector* pConnConnector);

	//Cancel connect call, stop the handshake if connection hasn't made
	void CancelHandShake();
protected:
	//Internal functions	
	virtual void OnRecvConnResp();
	virtual void OnRecvDisconn();
public:
	CPkgConnClient();
	virtual ~CPkgConnClient();
protected:
	int m_nConnReqCnt;	//Counter for ConnReq PDU send
	CPkgConnConnector* m_pConnConnector;			//Only Client use
	BOOL m_bConnRespRecved;

	BOOL m_bConnectOnceMade;
	BOOL m_bHandShakeCancelled;
};

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

class QT_OS_EXPORT CPkgConnServer : public CPkgConn
{
public:
	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId);
	
	virtual void OnTimer(CQtTimerWrapperID* aId);

	// interface IQtAcceptorConnectorSink
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId);
public:
	//Unconnected time stamp
	CQtTimeValue GetDisconnTimestamp();
public:
	CPkgConnServer();
	virtual ~CPkgConnServer();

	//In order to keep one instance of Server List.
	void SetServerList(CPkgConnServerList *pSvrList);
	void SetConnAcceptor(CPkgConnAcceptor* pConnAcceptor);
protected:
	virtual void OnRecvConnReq();
	void Reset();
	
protected:
	//CPkgConnServerList* m_pSvrList;
	CQtComAutoPtr<CPkgConnAcceptor> m_pConnAcceptor;

	//CQtTimeValue m_disconn_timestamp;
//	BOOL m_bConnReqRecved;

	CQtTimerWrapperID m_ReleaseTimer;
};
#endif
#endif	//PKG_H
