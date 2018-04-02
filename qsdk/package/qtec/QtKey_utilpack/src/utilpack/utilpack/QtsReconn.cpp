
#ifdef _NEW_PROTO_TP

#include "QtsReconn.h"

#define INIT_RECONNECT_INTERVAL		((LONG)1)			//reconnect interval /s
#define TIMEOUT_FOR_CONNECTION		((LONG)10)			//connect timeout


CBlockBack::CBlockBack()
{
	m_dwLatestPacketSizeBak = 0;
	m_PDUTypeBak = CS_PDU_TYPE_INVALID;
}

CBlockBack::~CBlockBack()
{}

void CBlockBack::Backup(TBuff &rBuff
						, PACKET_TYPE &type
						, DWORD &dwLatestPacketSize)
{
	m_RcvBak = rBuff;
	m_PDUTypeBak = type;
	type = CS_PDU_TYPE_INVALID;
	m_dwLatestPacketSizeBak = dwLatestPacketSize;
	dwLatestPacketSize = 0;
}

void CBlockBack::Restore(TBuff &rBuff
						 , PACKET_TYPE &type
						 , DWORD &dwLatestPacketSize)
{
	rBuff = m_RcvBak;
	type = m_PDUTypeBak;
	dwLatestPacketSize = m_dwLatestPacketSizeBak;
	m_dwLatestPacketSizeBak = 0;
	m_PDUTypeBak = CS_PDU_TYPE_INVALID;
}

//////////////////////////////////////////////////////////////////////////


CReconnClient::CReconnClient(IConnectorT *pConnector)
: CPacketClient(pConnector)
, m_ReconnectTick(0)
, m_ReconnectInterval(INIT_RECONNECT_INTERVAL)
{
	m_LinkType = REC_LINK;
	m_dwRetryTimes = 0;
	m_bRetryTimerStarted = FALSE;
}

CReconnClient::~CReconnClient()
{
	QT_STATE_TRACE_THIS("CReconnClient::~CReconnClient()");
}

void CReconnClient::OnDisconnect(QtResult aReason,	IQtTransport *pTransport)
{
	QT_STATE_TRACE_THIS("CReconnClient::OnDisconnect transport = " <<	pTransport 
		<< " Reason = " << aReason << " Status = " << m_LinkStatus);

	QT_ASSERTE(m_pITransport == pTransport);
	CQtComAutoPtr<CReconnClient> self(this);
	Backup(m_RcvBuff, m_PDUType, m_dwLatestPacketSize);
	if(m_pmbSendData)
	{
		m_pmbSendData->DestroyChained();
		m_pmbSendData = NULL;
	}
	
	if(m_pITransport.Get())
	{
		m_pITransport->Disconnect(QT_ERROR_NETWORK_CONNECT_ERROR);
		m_pITransport = NULL;
	}
	
	///for checkpint issue 2006.6.1
	if(!m_bRetryTimerStarted)
	{
		m_RetryCountTimer.Schedule(this, CQtTimeValue((double)COUNT_TIME), 1);
		m_bRetryTimerStarted = TRUE;
	}
	else
		++m_dwRetryTimes;
	if(m_dwRetryTimes >= RETRY_TIMES && m_TransportType != CQtConnectionManager::CTYPE_UDP) //abort to retry
	{
		m_bRetryTimerStarted = FALSE;
		m_RetryCountTimer.Cancel();
		QT_INFO_TRACE_THIS("CReconnClient::OnDisconnect(), the connection be reset too continually, abort it");
		m_LinkStatus = 	LINK_INACTIVE_STATUS;	
		
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(QT_ERROR_NETWORK_RETRY_ERROR, this);
		return;
	}
	
	
	if(m_bCancelConnectFlag)
		return;

	if(!m_bGetDisconnectPDU && LINK_PREP_STATUS != m_LinkStatus)
	{ 
		if(m_ReconnectTick.latest_tag() == 0)
			m_ReconnectTick.reset();
		
		if(/*m_ReconnectTick.overtime_sec(m_dwAbateTime)*/
			m_LatestRcvTick.overtime_sec(m_dwAbateTime))//modified, 2006.7.19, overtime
			///time out
		{
			QT_WARNING_TRACE_THIS("CReconnClient::OnDisconnect reconnect time out");
			m_pITransportSink->OnDisconnect(aReason, this);
		}
		else
		{
			QT_STATE_TRACE_THIS("CReconnClient::OnDisconnect reconnect again");
			m_ReconnectTimer.Schedule(this, m_ReconnectInterval, 1);
			if(LINK_ACTIVE_STATUS == m_LinkStatus)
				m_LinkStatus = LINK_PENDING_STATUS;
			m_ReconnectInterval *= 2;
		}
	}
	else 
	{
		if(LINK_PREP_STATUS == m_LinkStatus)		//ondisconnect before shakehand first time
			m_pConnector->OnConnectIndication(QT_ERROR_NETWORK_CONNECT_ERROR, NULL, m_pConnector);
		else //disconnect by server after shakehand
			m_pITransportSink->OnDisconnect(aReason, this);
		m_LinkStatus = 	LINK_INACTIVE_STATUS;	
		m_bGetDisconnectPDU = FALSE;
	}

}


void CReconnClient::Reconnect()
{
	QT_STATE_TRACE_THIS("CReconnClient::Reconnect(), timer elapsed " << m_ReconnectTick.elapsed_sec() << "s");
	m_pConnector->CancelConnect();
	m_bCancelConnectFlag = FALSE;
	CQtTimeValue tvTimeOut(TIMEOUT_FOR_CONNECTION);
	m_pConnector->AsycReconnect(this, &tvTimeOut);//GetTPConnector()->AsycConnect(this, m_pConnector->GetPeerAddr(), &tvTimeOut);
}

void CReconnClient::OnTimer(CQtTimerWrapperID *aId)
{
	//for Checkpoint issue, that 80 must be http packets
	if(aId == &m_RetryCountTimer)
	{
		if(m_dwRetryTimes < RETRY_TIMES) //allow TCP connect
			m_dwRetryTimes = 0;
		else
			QT_ASSERTE(FALSE);
		m_bRetryTimerStarted = FALSE;
		return;
	}
	if(&m_ReconnectTimer == aId){
		Reconnect();
	}
	else
		CPacketClient::OnTimer(aId);
}

void CReconnClient::OnConnectIndication(
					QtResult aReason,
					IQtTransport *pTransport,
					IQtAcceptorConnectorId *aRequestId)
{
	PktConnectionWithKeepAlive_T::OnConnectIndication(aReason, pTransport, aRequestId);
	QT_STATE_TRACE_THIS("CReconnClient::OnConnectIndication aReason = " << aReason << 
		" transport = " << pTransport << " request = " << aRequestId );
	
	if(m_bCancelConnectFlag)
	{
		Disconnect(QT_ERROR_NETWORK_DENY_ERROR);
		QT_INFO_TRACE_THIS("CReconnClient::OnConnectIndication the connection already be cancel");
		m_bCancelConnectFlag = FALSE;
		return;
	}
	
	if(QT_SUCCEEDED(aReason)) ///send request
	{
		m_ReconnectInterval = INIT_RECONNECT_INTERVAL; //reset the reconnect interface
		if(QT_FAILED(HandshakeRequest(m_LinkType))) ///send request
		{
			if(LINK_PREP_STATUS == m_LinkStatus)
				m_pConnector->OnConnectIndication(QT_ERROR_NETWORK_CONNECT_ERROR, NULL, m_pConnector);
			else  ///reconnect case
			{
				QT_WARNING_TRACE_THIS("CReconnClient::OnConnectIndication  reconnect shake hand failed, reason = " << aReason);
				m_pITransportSink->OnDisconnect(QT_ERROR_NETWORK_CONNECT_ERROR, this);
				Disconnect(QT_ERROR_NETWORK_CONNECT_ERROR);
			}
		}
		m_ReconnectTick.reset(0);
	}
	else
	{
		if(LINK_INACTIVE_STATUS == m_LinkStatus) ///first connect case
		{
			QT_WARNING_TRACE_THIS("CReconnClient::OnConnectIndication  failed");
			m_pConnector->OnConnectIndication(aReason, NULL, m_pConnector);
		}
		else  ///reconnect case
		{
			if(/*m_ReconnectTick*/m_LatestRcvTick.overtime_sec(m_dwAbateTime) || QT_ERROR_NETWORK_NO_SERVICE == aReason)
			{
				QT_WARNING_TRACE_THIS("CReconnClient::OnConnectIndication  reconnect time out or service not available and failed");
				if(m_pITransportSink)
					m_pITransportSink->OnDisconnect(aReason, this);
				m_LinkStatus = LINK_INACTIVE_STATUS;
			}
			else {
				m_pConnector->CancelConnect();
				m_ReconnectTimer.Schedule(this, m_ReconnectInterval, 1);
				if(LINK_ACTIVE_STATUS == m_LinkStatus)
					m_LinkStatus = LINK_PENDING_STATUS;
				m_ReconnectInterval *= 2;
			}
		}
	}
}


void CReconnClient::OnReceiveConnectResponse(CRespPDU &responsePDU)
{
	LINK_STATUS latestStatus = m_LinkStatus;
	CPacketClient::OnReceiveConnectResponse(responsePDU);
	if(latestStatus == LINK_BLOCK_STATUS && LINK_ACTIVE_STATUS == m_LinkStatus)
	{
		PktConnectionWithKeepAlive_T::OnSend(this);
	}
}

QtResult CReconnClient::SetOption(DWORD aCommand, LPVOID aArg)
{
	switch(aCommand) {
	case CS_OPT_RECONNECT_TIME:
		QT_ASSERTE_RETURN(*static_cast< LONG * >(aArg) > 0, QT_ERROR_FAILURE);
		m_ReconnectInterval = *static_cast< LONG * >(aArg);
		break;
	default:
		return CPacketClient::SetOption(aCommand, aArg);
	}
	return QT_OK;
}	
QtResult CReconnClient::GetOption(DWORD aCommand, LPVOID aArg)
{
	switch(aCommand) {
	case CS_OPT_RECONNECT_TIME:
		*static_cast< LONG * >(aArg) = m_ReconnectInterval;
		break;
	default:
		return CPacketClient::GetOption(aCommand, aArg);
	}
	return QT_OK;
}
//////////////////////////////////////////////////////////////////////////

#if defined (USE_SOCKETSERVER)
CReconnServer::CReconnServer(IQtTransport *pTransport
							 , ConnectionList_T *pConnectionList
							 , IQtTransportSink* pITransportSink)
: CPacketServer(pTransport, pConnectionList, pITransportSink)
{
	m_LinkType = REC_LINK;
}

CReconnServer::~CReconnServer()
{
	QT_STATE_TRACE_THIS("CReconnServer::~CReconnServer()");
}

void CReconnServer::OnDisconnect(QtResult aReason,	IQtTransport *pTransport)
{
	QT_INFO_TRACE_THIS("CReconnServer::OnDisconnect transport = " <<	pTransport 
		<< " Reason = " << aReason << " Status = " << m_LinkStatus);
	
	QT_ASSERTE(m_pITransport == pTransport);
	CQtComAutoPtr<CReconnServer> self(this);
	Backup(m_RcvBuff, m_PDUType, m_dwLatestPacketSize);
	if(m_pmbSendData)
	{
		m_pmbSendData->DestroyChained();
		m_pmbSendData = NULL;
	}
	if(m_pITransport.Get())
	{
		m_pITransport->Disconnect(QT_ERROR_NETWORK_CONNECT_ERROR);
		m_pITransport = NULL;
	}
	if(m_bGetDisconnectPDU)
		//disconnect by server in the course of naturenormally
	{
		m_LinkStatus = 	LINK_INACTIVE_STATUS;	
		if(m_pITransportSink)
		{
			m_pITransportSink->OnDisconnect(aReason, this);
			m_pITransportSink = NULL;
		}
	}
	else 
	{
		QT_INFO_TRACE_THIS("CReconnServer::OnDisconnect waiting for client reconnect");
		m_LinkStatus = 	LINK_PENDING_STATUS;	
		m_AbateTimer.Schedule(this, LINK_ABATE_TIME);
	}
}

QtResult CReconnServer::Disconnect(QtResult aReason)
{
	QT_STATE_TRACE_THIS("CReconnServer::Disconnect");
	m_pConnectionList->RemoveConnection(this);
	return CPacketServer::Disconnect(aReason);
}


void CReconnServer::OnTimer(CQtTimerWrapperID *aID)
{
	if(m_wDetectKeepAliveTimes >= MAX_KEEPALIVE_DETECT) 	///the connection is overtime, may be already break, destroy my self
	{
		OnDisconnect(QT_ERROR_NETWORK_CONNECT_ERROR, m_pITransport.Get());
		m_wDetectKeepAliveTimes = 0;
	}
	else CPacketServer::OnTimer(aID);
}
#endif //USE_SOCKETSERVER
#endif
