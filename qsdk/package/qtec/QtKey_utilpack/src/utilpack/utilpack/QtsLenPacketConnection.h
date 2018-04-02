/*------------------------------------------------------*/
/* the length package connection class                  */
/*                                                      */
/* CsLenPacketConnection.h                              */
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

#if !defined _LENPACKET_CONNECTION_H && defined _NEW_PROTO_TP
#define _LENPACKET_CONNECTION_H

#include "QtsPacketConnection_T.h"
#include "QtsPacketPDU.h"

typedef CPacketConnection_T<CLenPacketPDU, false>   CLenPacketConnection_t;

class CLenPacketConnection: public CLenPacketConnection_t
{
public:
	CLenPacketConnection(IConnectorT *pConnector)
		: CLenPacketConnection_t(CLIENT_CONNECTION, LEN_PKG_LINK)
		, m_pConnector(pConnector)
	{}

	CLenPacketConnection(CAcceptorT<CLenPacketConnection> *pAcceptor, LINK_TYPE type)
		: CLenPacketConnection_t(SERVER_CONNECTION, LEN_PKG_LINK)
		, m_pAcceptor(pAcceptor)
	{}


	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId)
	{
		QT_STATE_TRACE_THIS("CLenPacketConnection::OnConnectIndication, intance type = " << m_ConnectionType);
		CLenPacketConnection_t::OnConnectIndication(aReason, pITransport, aRequestId);
		if(QT_SUCCEEDED(aReason))
		{
			m_LinkStatus = LINK_ACTIVE_STATUS;
			CQtConnectionManager::CType lTransport;
			QtResult result = pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &lTransport);
			QT_ASSERTE(QT_SUCCEEDED(result));
			m_TransportType = lTransport;
		}
		QT_ASSERTE(INVALID_CONNECTION != m_ConnectionType);
		if(m_ConnectionType == CLIENT_CONNECTION)
			m_pConnector->OnConnectIndication(aReason, this, m_pConnector);
		else
			m_pAcceptor->OnConnectIndication(aReason, this, m_pAcceptor.Get());
	}
		

	QtResult Disconnect(QtResult aReason = 0)
	{
		if(m_pITransport.Get())
			m_pITransport->Disconnect(0);
		m_LinkStatus = LINK_INACTIVE_STATUS;
		m_pITransport = NULL;
		m_pITransportSink = NULL;
		Clear();
		return QT_OK;
	}
		
private:
	IConnectorT			*m_pConnector;
	CQtComAutoPtr< CAcceptorT<CLenPacketConnection> > m_pAcceptor;
};

#endif

