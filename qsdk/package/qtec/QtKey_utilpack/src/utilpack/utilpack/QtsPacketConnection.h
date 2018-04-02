/*------------------------------------------------------*/
/* the package connection class						    */
/*                                                      */
/* CsPacketConnection.h                                 */
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

#if !defined _PACKET_CONNECTION_H && defined _NEW_PROTO_TP
#define _PACKET_CONNECTION_H

#include "QtsPacketConnection_T.h"
#include "QtsPacketPDU.h"

class CPacketClient :	public CPacketConnection_T<CDataPktPDU, true>
{
public:
	CPacketClient(IConnectorT *pConnector);
	virtual ~CPacketClient();

	virtual void OnConnectIndication(QtResult aReason, IQtTransport *aTrpt,IQtAcceptorConnectorId *aRequestId);
	QtResult HandshakeRequest(LINK_TYPE type);
	void CancelConnect();
protected:
	void OnTimer(CQtTimerWrapperID *aId);
	virtual void OnReceiveConnectResponse(CRespPDU &responsePDU);
	virtual void OnDisconnect(QtResult aReason, IQtTransport *pTransport);
		
protected:	
	IConnectorT			*m_pConnector;
	BOOL				m_bCancelConnectFlag;
	CQtTimerWrapperID	m_HandShakeTimer;
	LONG				m_lHandShakeInterval;
};

#if defined (USE_SOCKETSERVER)
class CPacketServer :	public PktConnectionWithKeepAlive_T
{
public:
	CPacketServer(IQtTransport *pTransport
		, ConnectionList_T *pConnectionList
		, IQtTransportSink* pITransportSink);
	virtual ~CPacketServer();

		
protected:
	virtual void OnTimer(CQtTimerWrapperID* aId);
	virtual void OnReceiveConnectRequest(CReqPDU &requestPDU);

		
protected:
	ConnectionList_T	*m_pConnectionList;
};
#endif
#endif

