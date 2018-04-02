
#ifdef _NEW_PROTO_TP

#include "QtsPacketConnection.h"
#include "QtsPacketPDU.h"

#define		CONNECT_TIME			((LONG)75)

CPacketClient::CPacketClient(IConnectorT *pConnector)
: CPacketConnection_T<CDataPktPDU, true>(CLIENT_CONNECTION, PKG_LINK)
, m_pConnector(pConnector), m_bCancelConnectFlag(FALSE)
{
	m_lHandShakeInterval = 2;
	m_AbateTimer.Schedule(this, CONNECT_TIME, 1); ///link time out is 30s
}

CPacketClient::~CPacketClient()
{
	QT_STATE_TRACE_THIS("CPacketClient::~CPacketClient() status = " << m_LinkStatus << 
		" transport = " << m_pITransport.Get());
	if(m_LinkStatus != LINK_INACTIVE_STATUS) //connection has been not destory
	{
		Disconnect();
	}
}

void CPacketClient::OnConnectIndication(
								 QtResult aReason,
								 IQtTransport *pTransport,
								 IQtAcceptorConnectorId *aRequestId)
{
	CPacketConnection_T<CDataPktPDU, true>::OnConnectIndication(aReason, pTransport, aRequestId);
	QT_STATE_TRACE_THIS("CPacketClient::OnConnectIndication aReason = " << aReason << 
		" transport = " << pTransport << " request = " << aRequestId );

	if(!m_bCancelConnectFlag)
	{
		m_RTTDetectTick.reset();
		if(QT_FAILED(aReason) || 
			QT_FAILED(HandshakeRequest(m_LinkType))) ///send request
		{
			m_pConnector->OnConnectIndication(
				aReason == 0 ? QT_ERROR_NETWORK_CONNECT_ERROR : aReason, NULL, m_pConnector);
		}
	}
	else
	{
		QT_INFO_TRACE_THIS("CPacketClient::OnConnectIndication the connection already be cancel");
		Disconnect(QT_ERROR_NETWORK_DENY_ERROR);
		m_bCancelConnectFlag = FALSE;
	}
}

QtResult CPacketClient::HandshakeRequest(LINK_TYPE type)
{
	QT_ASSERTE(!m_bCancelConnectFlag);
	QtResult result = QT_OK;
	CReqPDU requestPDU(type, m_wConnectionID, RLB_LINK == type, GetACKSequence());
	CQtMessageBlock mb(requestPDU.GetFixLength(CS_PDU_TYPE_CONN_REQ, RLB_LINK == type));
	if( QT_SUCCEEDED(requestPDU.Encode(mb)) )
	{
		result = m_pITransport->SendData(mb);
//		if(m_TransportType == CQtConnectionManager::CTYPE_UDP) //try some times until abate timer overflow
//		{
			m_lHandShakeInterval *= 2;
			m_HandShakeTimer.Schedule(this, m_lHandShakeInterval/*CQtTimeValue(0, m_lHandShakeInterval * 100000)*/, 1);
//		}
	}
	else
	{
		QT_ERROR_TRACE_THIS("CPacketClient::OnConnectIndication request pdu ecode failed!");
		return QT_ERROR_FAILURE;
	}
	return result;
}

void CPacketClient::OnReceiveConnectResponse(CRespPDU &responsePDU)
{
	QT_STATE_TRACE_THIS("CPacketClient::OnReceiveConnectResponse resp tag = " << responsePDU.GetTag() << " ack = " << responsePDU.GetAck());
	m_dwRTT = m_RTTDetectTick.elapsed();
	m_AbateTimer.Cancel();
	m_HandShakeTimer.Cancel();

	if(m_bCancelConnectFlag || responsePDU.GetType() != m_LinkType)
	{
		Disconnect(QT_ERROR_NETWORK_DENY_ERROR);
		if(m_bCancelConnectFlag){
			QT_WARNING_TRACE_THIS("CPacketClient::OnReceiveConnectResponse the connection already be cancel");
		}
		else {
			QT_WARNING_TRACE_THIS("CPacketClient::OnReceiveConnectResponse the connection type is mismatch, accept type = " << responsePDU.GetType() << " My type = " << m_LinkType);
		}
		m_bCancelConnectFlag = FALSE;
		return;
	}
	
	if(m_wConnectionID != 0 )
	{
		QT_ASSERTE(responsePDU.GetTag() == m_wConnectionID);
	}
	m_wConnectionID = responsePDU.GetTag();
	LINK_TYPE LatestStatus = m_LinkStatus;
	m_LinkStatus = LINK_ACTIVE_STATUS;
	if(m_pConnector && LatestStatus ==LINK_PREP_STATUS)
	{
		m_pConnector->OnConnectIndication(QT_OK, this, m_pConnector);
	}
}

void CPacketClient::OnTimer(CQtTimerWrapperID *aId)
{
	if(aId == &m_AbateTimer || 
		(aId == &m_HandShakeTimer && QT_FAILED (HandshakeRequest(m_LinkType))) )
	{
		BOOL latestCancelConnectFlag = m_bCancelConnectFlag;
		m_HandShakeTimer.Cancel();
		if(m_pConnector)
		{
			m_pConnector->CancelConnect();
		}
		if(!latestCancelConnectFlag && LINK_PREP_STATUS == m_LinkStatus && m_pConnector)
		{
			m_pConnector->OnConnectIndication(QT_ERROR_FAILURE, NULL, m_pConnector);
		}
	}
	else
		PktConnectionWithKeepAlive_T::OnTimer(aId);
}


void CPacketClient::OnDisconnect(QtResult aReason, IQtTransport *pTransport)
{
	if(LINK_PREP_STATUS == m_LinkStatus && !m_bCancelConnectFlag && m_pConnector)
	{
		m_pConnector->OnConnectIndication(QT_ERROR_FAILURE, NULL, m_pConnector);
		m_LinkStatus = LINK_INACTIVE_STATUS;
	}
	PktConnectionWithKeepAlive_T::OnDisconnect(aReason, pTransport);
}


void CPacketClient::CancelConnect()
{
	if(m_pConnector)
		m_pConnector->CancelConnect();
	m_HandShakeTimer.Cancel();
	m_AbateTimer.Cancel();
	m_bCancelConnectFlag = TRUE;
}

//////////////////////////////////////////////////////////////////////////

#if defined (USE_SOCKETSERVER)
CPacketServer::CPacketServer(IQtTransport *pTransport
							 , ConnectionList_T *pConnectionList
							 , IQtTransportSink* pITransportSink)
: CPacketConnection_T<CDataPktPDU, true>(SERVER_CONNECTION, PKG_LINK)
, m_pConnectionList(pConnectionList)
{
	m_pITransport = pTransport;
	QT_ASSERTE(m_pITransport);
	QT_ASSERTE(m_pConnectionList);
	m_pITransportSink = pITransportSink;
	m_pITransport->OpenWithSink(this);
	m_LinkStatus = LINK_ACTIVE_STATUS;
}

CPacketServer::~CPacketServer()
{
	QT_STATE_TRACE_THIS("CPacketServer::~CPacketServer() status = " << m_LinkStatus << 
		" transport = " << m_pITransport.Get());
	if(m_LinkStatus != LINK_INACTIVE_STATUS) //connection has been not destory
	{
		Disconnect();
	}
}

void CPacketServer::OnReceiveConnectRequest(CReqPDU &requestPDU)
{
	QT_STATE_TRACE_THIS("CPacketServer::OnReceiveConnectRequest request type = " << requestPDU.GetType() 
		<< " request ID = " << requestPDU.GetTag() << " status = " << m_LinkStatus);
	
	QT_ASSERTE(requestPDU.GetType() == m_LinkType);
	CRespPDU respPdu(m_wConnectionID, RLB_LINK == m_LinkType, m_LinkType);
	CQtMessageBlock mb(respPdu.GetFixLength(CS_PDU_TYPE_CONN_RESP, RLB_LINK == m_LinkType));
	if( QT_FAILED(respPdu.Encode(mb)) || QT_FAILED(m_pITransport->SendData(mb)) )
	{
		QT_ERROR_TRACE_THIS("CConnectionAdaptor::OnReceiveConnectRequest response failed!");
	}
	m_LatestRcvTick.reset();
	m_LinkStatus = LINK_ACTIVE_STATUS;
}


void CPacketServer::OnTimer(CQtTimerWrapperID *aId)
{
	if(aId == &m_AbateTimer || 
		(m_wDetectKeepAliveTimes >= MAX_KEEPALIVE_DETECT && LINK_ACTIVE_STATUS == m_LinkStatus)) 	//abate timer overflow
	{
		QT_INFO_TRACE_THIS("CPacketServer::OnTimer abate timer overflow");
		m_pConnectionList->RemoveConnection(this);
	}
	PktConnectionWithKeepAlive_T::OnTimer(aId);
}
#endif //USE_SOCKETSERVER
#endif
