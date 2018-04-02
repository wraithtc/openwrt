/*------------------------------------------------------*/
/* the reconnect connection class definition		    */
/*                                                      */
/* CsReconn.h                                           */
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

#if !defined RECONNECT_CONNECTION_H && defined _NEW_PROTO_TP
#define RECONNECT_CONNECTION_H

#include "QtsPacketConnection.h"

class CBlockBack
{
protected:
	TBuff m_RcvBak;
	PACKET_TYPE m_PDUTypeBak;
	DWORD m_dwLatestPacketSizeBak;
	
	void Backup(TBuff &rBuff
		, PACKET_TYPE &type
		, DWORD &dwLatestPacketSize);
	void Restore(TBuff &rBuff
		, PACKET_TYPE &type
		, DWORD &dwLatestPacketSize);
	CBlockBack();
	~CBlockBack();
};
//////////////////////////////////////////////////////////////////////////


class CReconnClient:public CPacketClient, public CBlockBack
{
	//Modify 2006.6.1, if TCP(80 port) connection break times over RETRY_TIMES in COUNT_TIME, abort it
	enum{
		RETRY_TIMES = 5,			
		COUNT_TIME	= 60			//seconds
	};
	
public:
	CReconnClient(IConnectorT *pConnector);
	virtual ~CReconnClient();
	virtual void OnDisconnect(QtResult aReason,	IQtTransport *pTransport);
	virtual void OnConnectIndication(QtResult aReason, IQtTransport *aTrpt,IQtAcceptorConnectorId *aRequestId);
	QtResult SetOption(DWORD aCommand, LPVOID aArg);
	QtResult GetOption(DWORD aCommand, LPVOID aArg);
	
protected:
	void Reconnect();
	void OnTimer(CQtTimerWrapperID *aId);
	virtual void OnReceiveConnectResponse(CRespPDU &responsePDU);
	
protected:
	CQtTimerWrapperID		m_ReconnectTimer;
	ticker					m_ReconnectTick;
	LONG					m_ReconnectInterval;

	//for checkpint 2006.6.1 Victor
	CQtTimerWrapperID	m_RetryCountTimer;
	DWORD	m_dwRetryTimes;
	BOOL	m_bRetryTimerStarted;
};

#if defined (USE_SOCKETSERVER)
class CReconnServer:public CPacketServer, public CBlockBack
{
public:
	CReconnServer(IQtTransport *pTransport
		, ConnectionList_T *pConnectionList
		, IQtTransportSink* pITransportSink);
	virtual ~CReconnServer();

	virtual void OnDisconnect(QtResult aReason,	IQtTransport *pTransport);
	QtResult Disconnect(QtResult aReason = 0);

protected:
	void OnTimer(CQtTimerWrapperID *aID);

};
#endif //USE_SOCKETSERVER
#endif
