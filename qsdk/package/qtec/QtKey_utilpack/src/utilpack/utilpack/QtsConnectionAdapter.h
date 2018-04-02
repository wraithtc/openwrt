
/*------------------------------------------------------*/
/* the adapter class									*/
/* work with server side to  fit client connection		*/
/* request												*/
/*                                                      */
/* CsConnectionAdapter.h                                */
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

#if !defined CONNECTION_ADAPTER_H && defined _NEW_PROTO_TP
#define CONNECTION_ADAPTER_H

#include "QtsPacketConnection.h"
#include "QtsRlb.h"

#if defined (USE_SOCKETSERVER)
class CConnectionFactory //only for pkg, sender pkg, reconnect, rlb connection
{
public:
	static PktConnectionWithKeepAlive_T *CreateConnection
		(LINK_TYPE type
		, IQtTransport *pTransport
		, ConnectionList_T *pConnectionList
		, IQtTransportSink* pITransportSink);
};

class CConnectionAdaptor 
	: public CPacketConnection_T<CDataPktPDU, true>
{
public:
	CConnectionAdaptor(CAcceptorT<CConnectionAdaptor> *pAcceptor, 
		LINK_TYPE type = ANY_LINK);
	
	void SetAcceptorType(LINK_TYPE acceptType);
protected:
	virtual void OnReceiveConnectRequest(CReqPDU &requestPDU);

private:
	CQtComAutoPtr< CAcceptorT<CConnectionAdaptor> > m_pAcceptor;
	LINK_TYPE  m_AcceptLinkType;
	static ConnectionList_T	m_ConnectionList;
};
#endif //USE_SOCKETSERVER
#endif
