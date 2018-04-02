
#ifdef _NEW_PROTO_TP
#include "QtsConnectionAdapter.h"

#if defined (USE_SOCKETSERVER)
ConnectionList_T	CConnectionAdaptor::m_ConnectionList;

PktConnectionWithKeepAlive_T * CConnectionFactory::
	CreateConnection(LINK_TYPE type,
	IQtTransport *pTransport, 
	ConnectionList_T *pConnectionList,
	IQtTransportSink* pITransportSink)
{
	QT_ASSERTE_RETURN(RLB_LINK == type || 
		PKG_LINK == type || 
		REC_LINK == type, NULL);

	PktConnectionWithKeepAlive_T *pConnection = NULL;
	switch(type) {
	case PKG_LINK:
		pConnection = new CPacketServer(pTransport, pConnectionList, pITransportSink);
		break;
	case REC_LINK:
		pConnection = new CReconnServer(pTransport, pConnectionList, pITransportSink);
		break;
	case RLB_LINK:
		pConnection = new CRLBServer(pTransport, pConnectionList, pITransportSink);
		break;
	}
	return pConnection;
}




CConnectionAdaptor::
CConnectionAdaptor(CAcceptorT<CConnectionAdaptor> *pAcceptor, 
				   LINK_TYPE type)
: CPacketConnection_T<CDataPktPDU, true>(SERVER_CONNECTION, PKG_LINK)
, m_pAcceptor(pAcceptor), m_AcceptLinkType(type)
{
	m_AbateTimer.Schedule(this, (LONG)m_dwAbateTime, 1);
}


void CConnectionAdaptor::OnReceiveConnectRequest(CReqPDU &reqPdu)
{
	QT_STATE_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest request type = " << reqPdu.GetType() 
		<< " now support type = " << m_AcceptLinkType << " request ID = " 
		<< reqPdu.GetTag() << " status = " << m_LinkStatus);

	QT_ASSERTE_RETURN_VOID(m_LinkStatus != LINK_INACTIVE_STATUS);

	CQtComAutoPtr<CConnectionAdaptor> self(this);
	
	PktConnectionWithKeepAlive_T * pConnection;
	m_dwRTT = m_RTTDetectTick.elapsed() * 2;
	
	if(reqPdu.GetType() & m_AcceptLinkType) //the requesting type is support
	{
		if(reqPdu.GetTag() == INVALID_CONNECTION_ID)	//the new connection request
		{
			pConnection = CConnectionFactory::CreateConnection
				(reqPdu.GetType(), m_pITransport.Get(), &m_ConnectionList, m_pITransportSink);

			WORD wConnectionID = 0;
			if(pConnection)
			{
				pConnection->SetOption(CS_OPT_NEED_KEEPALIVE, (LPVOID)&m_bNeedKeepAlive);
				if( reqPdu.GetType() == REC_LINK ||
					reqPdu.GetType() == RLB_LINK)
				{
					wConnectionID = m_ConnectionList.AddConnection(pConnection);
					if(INVALID_CONNECTION_ID == wConnectionID)
						///the connection list is overflow, deny the connection and destroy self
					{
						QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest the connection list is overflow, deny the connection and destroy self");
						Disconnect(QT_ERROR_NETWORK_DENY_ERROR);
						delete pConnection;
						return;
					}
				}
				pConnection->SetRTT(m_dwRTT);
				m_AbateTimer.Cancel();
				m_LinkStatus = LINK_ACTIVE_STATUS;
				QT_ASSERTE_RETURN_VOID(m_pAcceptor.Get());

				CRespPDU respPdu(wConnectionID, RLB_LINK == reqPdu.GetType(), reqPdu.GetType());
				CQtMessageBlock mb(respPdu.GetFixLength(CS_PDU_TYPE_CONN_RESP, RLB_LINK == reqPdu.GetType()));
				if( QT_FAILED(respPdu.Encode(mb)) || QT_FAILED(m_pITransport->SendData(mb)) )
				{
					QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest response failed!");
				}
				else
				{
					m_pAcceptor->OnConnectIndication(QT_OK, pConnection, m_pAcceptor.Get());
					pConnection->SetID(wConnectionID);
					m_pITransport = NULL;
				}
			}
			else
			{
				QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest out of memory or miss connection type, type = " 
					<< reqPdu.GetType());
				Disconnect(QT_ERROR_NETWORK_DENY_ERROR);
			}
		}
		else //must be reconnect request, get from connection list and attach to it
		{
			QT_ASSERTE_RETURN_VOID(REC_LINK == reqPdu.GetType() || RLB_LINK == reqPdu.GetType());
			pConnection = m_ConnectionList.GetConnection(reqPdu.GetTag()).Get();
			if(!pConnection) ///the connection can not be found, already destroy?
			{
				QT_WARNING_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest the reconnect connection can not be found");
				Disconnect(QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);
			}
			else //bind the raw TP to the connection
			{
				pConnection->SetRTT(m_dwRTT);
				CRespPDU respPdu(reqPdu.GetTag(), RLB_LINK == reqPdu.GetType(), reqPdu.GetType(), pConnection->GetACKSequence());
				CQtMessageBlock mb(respPdu.GetFixLength(CS_PDU_TYPE_CONN_RESP, RLB_LINK == reqPdu.GetType()));
				if( QT_FAILED(respPdu.Encode(mb)) || QT_FAILED(m_pITransport->SendData(mb)) )
				{
					pConnection->Disconnect();
					m_ConnectionList.RemoveConnection(pConnection);
					QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest failed!");
				}
				else
				{
					pConnection->Attach(m_pITransport.Get(), reqPdu.GetAck());
					m_pITransport = NULL;
				}
			}
		}
	}
	else //the request is no support
	{
		CRespPDU respPdu(reqPdu.GetTag(), RLB_LINK == reqPdu.GetType(), m_AcceptLinkType);
		CQtMessageBlock mb(respPdu.GetFixLength(CS_PDU_TYPE_CONN_RESP, RLB_LINK == reqPdu.GetType()));
		m_pITransport->SendData(mb);
		QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest the connection type is not allowed");
		Disconnect(QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE);
	}
}
#endif //USER_SOCKETSERVER
#endif
