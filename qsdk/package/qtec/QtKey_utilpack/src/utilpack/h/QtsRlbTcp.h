/*------------------------------------------------------*/
/* Reliable Connection with TCP classes                 */
/*                                                      */
/* CsRlbTcp.h                                           */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*      zhubin (zhubin@qtec.cn)                         */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

//$Id: CsRlbTcp.h,v 1.82.4.1 2010/03/29 10:14:28 jerryh Exp $

#if !defined RLB_TCP_H  && !defined (_NEW_PROTO_TP)
#define RLB_TCP_H

#include "QtsBase.h"

#define SEND_BUF_LEN_MAX4RLB		262144 		//128K bytes 

#define MAX_TIMES_NO_RECV			6			//For Keep Alive or Data recv Checking(MAX_TIMES_NO_RECV*INTERVAL2 = 1 Minute)
#define MAX_TRY_CONNECT				6			//For Conn-Resp Checking
#define TIMEOUT_FOR_CONNECTION		(double)10

class CRlbConnTCPClient;
class CRlbConnTCPServer;

typedef ServerListT<CRlbConnTCPServer>			CRlbConnTCPServerList;

typedef CConnAcceptorSinkT<CRlbConnTCPServer>	CRlbConnTCPAcceptorSink;
typedef CConnAcceptorT<CRlbConnTCPServer>		CRlbConnTCPAcceptor;

typedef CConnConnectorT<CRlbConnTCPClient>		CRlbConnTCPConnector;

//For Upper layer
//Act as a Client IQtTransport obj to Upper layer, 
//and also as a IQtTransportSink sink into TP layer.
class QT_OS_EXPORT CRlbConnTCPClient : public CCsConn
{
#if !defined QT_WIN32 && !defined QT_PORT_CLIENT
	//Modify 2009.3.9, for SPA on ssl gateway
	enum{
		RETRY_TIMES = 21,			
		COUNT_TIME	= 60			//seconds
	};
#else
	//Modify 2006.6.1, if TCP(80 port) connection break times over RETRY_TIMES in COUNT_TIME, abort it
	enum{
		RETRY_TIMES = 5,			
		COUNT_TIME	= 60			//seconds
	};
#endif
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

	virtual QtResult Disconnect(QtResult aReason);

	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg);
public:
	//Client will do Reconnection  
	void SetConnConnector(CRlbConnTCPConnector* pConnConnector);

	//Cancel connect call, stop the handshake if connection hasn't made
	void CancelHandShake();

protected:
	//Internal functions	
	virtual void Reconnect();
	virtual void OnRecvConnResp();
	virtual void OnRecvDisconn();
	void Reset();
	void Reset4Reconnect();

public:
	CRlbConnTCPClient(DWORD dwSendBuffSize = SEND_BUF_LEN_MAX4RLB);
	virtual ~CRlbConnTCPClient();
protected:
	int m_nUngetDataCnt;//Counter for times haven't received data
//	int m_nReconnect;	//Max times for reconnection

	CQtComAutoPtr<CRlbConnTCPConnector> m_pConnConnector;			//Only Client use
	BOOL m_bConnectOnceMade;	//Indicate if Connection once made
//	WORD m_wMaxReconn;			//Max times to reconnect

	BOOL m_bHandShakeCancelled;
	DWORD	m_dwDisconnTick;
	CQtTimerWrapperID	m_ReconnectTimer;
	//for checkpint 2006.6.1 Victor
	CQtTimerWrapperID	m_RetryCountTimer;
	DWORD	m_dwRetryTimes;
	BOOL	m_bRetryTimerStarted;
	BOOL	m_bStopFlag; //after call disconnect, this flags will be enable
};

//For Upper layer
//Act as a Server IQtTransport obj to Upper layer, 
//and also as a IQtTransportSink sink into TP layer.
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
#include <map>

class QT_OS_EXPORT CRlbConnTCPServer : public CCsConn
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

	virtual QtResult Disconnect(QtResult aReason);
	
	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg);
public:
	//Unconnected time stamp
	CQtTimeValue GetDisconnTimestamp();
	void TryOnDisconnIndicate();
protected:
	virtual void OnRecvConnReq();
	void Reset();
	void Reset4ReconnComing();
public:
	CRlbConnTCPServer(DWORD dwSendBuffSize = SEND_BUF_LEN_MAX4RLB);
	virtual ~CRlbConnTCPServer();

	//In order to keep one instance of Server List.
	void SetServerList(CRlbConnTCPServerList *pSvrList);

	void SetConnAcceptor(CRlbConnTCPAcceptor* pConnAcceptor);
protected:
	//When Reconnecting, new coming connection may 
	////Attach some handles & states to the old one
	void Attach(CRlbConnTCPServer* pSvr);
	void OnRecvKeepAlive();
protected:
	CRlbConnTCPServerList* m_pSvrList;
	CQtComAutoPtr<CRlbConnTCPAcceptor> m_pConnAcceptor;
	CQtTimerWrapperID m_ReleaseTimer;
	static DWORD m_sdwConnectionID;
};

class QtRlbServerLiveKeeper : public CQtTimerWrapperIDSink
{
	CQtTimerWrapperID m_Timer; //the timer to check the connection status
	std::list<CRlbConnTCPServer *> m_ConnectionsList; //use list, because the we focus the performance on timer
	std::list< CRlbConnTCPServer *>::iterator m_iter;
	QtRlbServerLiveKeeper();
	~QtRlbServerLiveKeeper();

	static QtRlbServerLiveKeeper *m_sInstance;
	typedef CQtMutexNullSingleThread MutexType;
	MutexType m_Mutex;
	static CQtMutexThreadRecursive m_sMutex;
public:
	static QtRlbServerLiveKeeper * Instance();
	QtResult Register(CRlbConnTCPServer * pLoadUpdate, DWORD dwID);
	QtResult Unregister(CRlbConnTCPServer *pLoadUpdate, DWORD dwID);
	// the timer to calculate load
	void OnTimer(CQtTimerWrapperID* aId);
	
};

#endif
#endif // RLB_TCP_H

