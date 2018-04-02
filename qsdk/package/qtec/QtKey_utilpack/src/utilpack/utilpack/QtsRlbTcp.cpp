//$Id: CsRlbTcp.cpp,v 1.113.4.6 2010/05/26 08:56:38 jerryh Exp $

#include "QtBase.h"
#include "QtsRlbTcp.h"
#include "QtTraceFromT120.h"
#if !defined (_NEW_PROTO_TP)

///////////////////////////////////////////
//class CRlbConnTCPClient
///////////////////////////////////////////
CRlbConnTCPClient::CRlbConnTCPClient(DWORD dwSendBuffSize) : CCsConn(dwSendBuffSize)
{
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::CRlbConnTCPClient");
	m_nUngetDataCnt = 0;

	m_cType = CQtConnectionManager::CTYPE_PDU_RELIABLE;
	m_bPDUNeedACK = dwSendBuffSize == 0 ? FALSE : TRUE;
	m_byConnType = CS_CONN_TYPE_RLB;

	m_bConnectOnceMade = FALSE;

	m_bHandShakeCancelled = FALSE;

	m_pConnConnector = NULL;
	m_byInstanceType = CLIENT_INSTANCE;
	m_dwDisconnTick = 0;
	m_dwRetryTimes = 0;
	m_bRetryTimerStarted = FALSE;
	m_bStopFlag = FALSE;
	m_bHandshake = FALSE;
}

CRlbConnTCPClient::~CRlbConnTCPClient()
{
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::~CRlbConnTCPClient()");
	Reset();
}

void CRlbConnTCPClient::Reset()
{
	m_nUngetDataCnt = 0;
	CCsConn::Reset();
}

void CRlbConnTCPClient::Reset4Reconnect()
{
	m_Timer.Cancel();

	m_nUngetDataCnt = 0;
	
	if(m_pmbLocData)
	{
		m_pmbLocData->DestroyChained();
		m_pmbLocData = NULL;
	}

	if(m_pConnReqPDU)
	{
		m_pConnReqPDU->DestroyChained();
		m_pConnReqPDU = NULL;
	}
	
	m_pITransport = NULL;//IQtTransport *aTrpt with OnConnectIndication() will be different when a new connection is made.

	Reset4Recv();
}

QtResult CRlbConnTCPClient::Disconnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::Disconnect(), aReason = " << aReason << 
		" m_pITransport = " << m_pITransport << 
		" sink = " << m_pITransportSink << 
		"m_bStopFlag = " << m_bStopFlag);
	
	if(m_bStopFlag) //the transport already be closed(disconnect) //fixed bug 390459
	{
		return QT_ERROR_NETWORK_SOCKET_CLOSE;
	}
	//////////////////////////////////////////////////////////////////////////
//	Reconnect(); //for verify bug, need remove in the release
	//////////////////////////////////////////////////////////////////////////
	m_Timer.Cancel();
	m_ReconnectTimer.Cancel();
	m_HandleTimer.Cancel();
	m_bStopFlag = TRUE;
	//////////////////////////////////////////////////////////////////////////
	m_bHandShakeCancelled = TRUE;
// 	if(m_pConnConnector)
// 		m_pConnConnector->CancelConnect();
	//////////////////////////////////////////////////////////////////////////
	CQtComAutoPtr<CRlbConnTCPClient> myself(this);
	QtResult result = QT_OK;
	if(m_pITransport && m_wStatus != STATUS_UNCONNECTED)
	{
		SendDisconn(aReason);
		result = m_pITransport->Disconnect(aReason);
		m_pITransport = NULL;
	}

	SetCurrStatus(STATUS_UNCONNECTED);

	m_pITransportSink = NULL;
	
	if(m_bConnectOnceMade)
		m_bConnectOnceMade = FALSE;
	m_dwDisconnTick = 0;

	return result;
}

//Client using
void CRlbConnTCPClient::OnRecvDisconn()
{

	//Get reason from Disconn PDU
	CCsPduDisconn pdc;
	CQtMessageBlock mb(pdc.GetFixLength());
	pdc.Decode(*m_pmbLocData);

	WORD wReason = pdc.GetReason();
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvDisconn(), wReason = " << wReason << 
		" m_pITransport = " << m_pITransport <<	" sink = " << m_pITransportSink << " Link Type = " << m_cBaseType);

	if(m_bStopFlag) //if already disconnect by upper layer, do nothing
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvDisconn(), connection has been cancelled.");
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_OK);
	}
	else if(QT_ERROR_NETWORK_RECEIVED_NONE == wReason) //server not get any packets over 30 seconds, failover Mar 23 2008 Victor
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvDisconn() server require reconnect, because it has not got any packets over 3 keep alive interval seconds");
		QT_ASSERTE(m_cBaseType == CQtConnectionManager::CTYPE_UDP);
		Reconnect();
		return;
	}
	//Waiting for peer do Disconnection
	m_bNormalDisconn = TRUE;

	m_Timer.Cancel();
	m_HandleTimer.Cancel();
	SetCurrStatus(STATUS_UNCONNECTED);
	if(m_pITransportSink)
		m_pITransportSink->OnDisconnect(QT_OK,this);
	//TCP will get OnDisconnect(..) callback later
}

void CRlbConnTCPClient::SetConnConnector(CRlbConnTCPConnector* pConnConnector)
{
	m_pConnConnector = pConnConnector;
};

void CRlbConnTCPClient::CancelHandShake()
{
	//While handshaking
	if(!m_bConnectOnceMade)
	{
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_OK);
		
		m_bHandShakeCancelled = TRUE;
	}
}

void CRlbConnTCPClient::OnConnectIndication(QtResult aReason, IQtTransport *aTrpt, IQtAcceptorConnectorId *aRequestId)
{
	QT_ASSERTE(m_pConnConnector->GetTPConnector() == aRequestId);
	m_HandleTimer.Cancel();
	if(m_bHandShakeCancelled)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication(), connection has been cancelled.");
		if(aTrpt)
			aTrpt->Disconnect((QtResult)QT_OK);

		return;
	}

	QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication(), m_pITransport = " << m_pITransport << ", aTrpt = " << 
		aTrpt << " reason = " << aReason << " used = " << (DWORD)m_Ticker.elapsed() << ", channelID = " << m_wChannel);
	m_Ticker.reset();

	m_pITransport = aTrpt;

	if(QT_SUCCEEDED(aReason))
	{
		//Get Transport type
		DWORD dwTransType = CQtConnectionManager::CTYPE_NONE;
		
		if(m_pITransport)
			m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
		
		m_cBaseType = dwTransType;
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication() connection type = " << m_cBaseType);
		
		m_pITransport = aTrpt;
		if(aTrpt)
		{
			DWORD dwConnectType = 0;
			m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &dwConnectType);
			if(dwConnectType & CQtConnectionManager::CTYPE_UDP)
			{
				m_dwMaxBuffLen = 0;
				m_SendBuf.SetMaxBufLen(m_dwMaxBuffLen);
			}
		}
		SetCurrStatus(STATUS_CONNECTED);//Physical connection make

		QT_ASSERTE_RETURN_VOID(m_pITransport);
		m_pITransport->OpenWithSink(this);

		SendConnReq();

		//m_Timer.schedule(for Timer1);waiting for connect response
		if(!m_bHandshake)
		{
			m_Timer.Schedule(this, CQtTimeValue(INTERVAL1, 0));
			m_bHandshake = TRUE;
		}

		if(m_cBaseType == CQtConnectionManager::CTYPE_UDP)
			m_HandleTimer.Schedule(this, CQtTimeValue(0, m_nInterval * 1000), 1);	

	}
	else //There are errors, such as timeouted
	{
		if(m_bConnectOnceMade)
		{
			if(m_LatestRcvTicker.overtime_sec((SERVER_UNAVAIL_TIMEOUT + 10)))
			{
				QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication(), Max time Reconnection timeout, times = " << m_LatestRcvTicker.elapsed_sec() << " now status = " <<  m_wStatus);
				if(m_pITransportSink)
					m_pITransportSink->OnDisconnect(aReason, this);
				SetCurrStatus(STATUS_UNCONNECTED);
			}
			else
			{
				m_ReconnectTimer.Cancel();
				QtResult rv = m_ReconnectTimer.Schedule(this, (long)5, 1);
				if(QT_FAILED(rv))
				{
					QT_ERROR_TRACE_THIS("Schedule timer failed! rv " << rv);
					OnDisconnect(aReason, this);
				}
			}
		}
		else if(m_bStopFlag) //for bug 308987, that cancel connect clear the status and lets it onconnectindication again, Victor 8/22 2008
			//the status should be not sure after diconnect the connection more than one times 9/10 2008
		{
			SetCurrStatus(STATUS_UNCONNECTED);
			m_ReconnectTimer.Cancel();
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication() the connection already disconnect by upper layer");
		}
		else
		{

			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnConnectIndication() connect failed");
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				aReason, 
				NULL, 
				m_pConnConnector.Get());
			SetCurrStatus(STATUS_UNCONNECTED);
		}
	}
}

void CRlbConnTCPClient::OnDisconnect(QtResult aReason,	IQtTransport *aTrptId)
{
	m_Ticker.reset();
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect aReason = " << aReason << " aTrptId = " << aTrptId << 
		" m_pITransport = " << m_pITransport << " m_pITransportSink = " << m_pITransportSink << " count = " << m_dwRetryTimes);
	QT_ASSERTE(m_pITransport == aTrptId);
	if(m_pmbRecData)
	{
		m_pmbRecData->DestroyChained();
		m_pmbRecData = NULL;
	}
	
	//Stop any action of sending
	m_Timer.Cancel();
	m_HandleTimer.Cancel();
		
	///for checkpint issue 2006.6.1
	if(!m_bRetryTimerStarted)
	{
		m_RetryCountTimer.Schedule(this, CQtTimeValue((double)COUNT_TIME), 1);
		m_bRetryTimerStarted = TRUE;
	}
	else
	{
#ifdef QT_LINUX
		//to fix 383506 to avoid wrong connect successful status on linux
		if(STATUS_DATA_CAN_SEND == m_wStatus)
#endif
		++m_dwRetryTimes;
	}
	if(m_dwRetryTimes >= RETRY_TIMES && m_cBaseType != CQtConnectionManager::CTYPE_UDP) //abort to retry
	{
		m_bRetryTimerStarted = FALSE;
		m_RetryCountTimer.Cancel();
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), the connection be reset too continually, abort it");
		SetCurrStatus(STATUS_UNCONNECTED);
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(QT_ERROR_NETWORK_RETRY_ERROR, this);
		return;
	}
	
	QT_ASSERTE_RETURN_VOID(m_pITransport);
	m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);

	if(aReason == QT_ERROR_NETWORK_SOCKET_BIND_ERROR ||
		aReason == QT_ERROR_NETWORK_CONNECT_ERROR ||
		aReason == QT_ERROR_NETWORK_CONNECT_TIMEOUT ||
		aReason == QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED /*
		|| 
		        aReason == QT_ERROR_NETWORK_PDU_ERROR*/
		)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), None Reconnection Disconnect");

		SetCurrStatus(STATUS_UNCONNECTED);

		//Callback ITcpRlbConnSink::OnDisconnect(Reason);
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(aReason, this);
	}
	else
	{
		if(m_bNormalDisconn)
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), After recv DisconnPDU");
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(aReason, this);
			
			return;
		}
		
//		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), Need Reconnection");
		
		if(m_bConnectOnceMade )
		{
			if(m_LatestRcvTicker.overtime_sec((SERVER_UNAVAIL_TIMEOUT + 10)))
			{
				QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), Max time Reconnection timeout, times = " << m_LatestRcvTicker.elapsed_sec() << " now status = " <<  m_wStatus);
				if(m_pITransportSink)
					m_pITransportSink->OnDisconnect(aReason, this);
				SetCurrStatus(STATUS_UNCONNECTED);
			}
			else
			{
				SetCurrStatus(STATUS_NEED_RECONNECT);

				if(m_cBaseType == CQtConnectionManager::CTYPE_UDP)
				{
					m_ReconnectTimer.Schedule(this, (long)5, 0);
				}
				else
				{
					m_ReconnectTimer.Schedule(this, (long)5, 1);
				}
			}
		}
		else if(m_bHandShakeCancelled || m_bStopFlag)
			//the status should be not sure after diconnect the connection more than one times 9/10 2008
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnDisconnect(), the connection already be cancelled");
			SetCurrStatus(STATUS_UNCONNECTED);
		}
		else
		{
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				aReason, 
				NULL, 
				m_pConnConnector.Get());
			SetCurrStatus(STATUS_UNCONNECTED);
		}
	}
}
	
void CRlbConnTCPClient::OnTimer(CQtTimerWrapperID* aId)
{
	if(aId == &m_HandleTimer)
	{
		m_nInterval *= 2;
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer interval = " << m_nInterval);
		m_HandleTimer.Schedule(this, CQtTimeValue(0, m_nInterval * 1000), 1);
		
		SendConnReq();//Resend ConnReq
		return;
	}
	
	//for Checkpoint issue, that 80 must be http packets
	if(aId == &m_RetryCountTimer)
	{
		if(m_dwRetryTimes < RETRY_TIMES) //allow TCP connect
			m_dwRetryTimes = 0;
		else
			QT_ASSERTE(FALSE);
		m_bRetryTimerStarted = FALSE;
		m_RetryCountTimer.Cancel();
		return;
	}
	else if(aId == &m_ReconnectTimer)
	{
		Reconnect();
		return;
	}
	if(m_wStatus == STATUS_CONNECTED)//For Timer1 of waiting for ConnResp 
	{
		if(m_wLastStatus == STATUS_NEED_RECONNECT) //if not get response but not overtime, try again
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer(), haven't recv ConnResp PDU reconnect again.");
			if(!m_LatestRcvTicker.overtime_sec((SERVER_UNAVAIL_TIMEOUT + 10)))
			{
				Reconnect();
				return;
			}
		}
		
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer(), haven't recv ConnResp PDU and timer out, elapsed = "<< m_LatestRcvTicker.elapsed_sec());
		if(m_pConnReqPDU)
		{
			m_pConnReqPDU->DestroyChained();
			m_pConnReqPDU = NULL;
		}
		
		m_Timer.Cancel();//Cancel Timer1
		m_HandleTimer.Cancel();
		m_ReconnectTimer.Cancel();
		m_nInterval = UDP_HAND_VAL;
		
		if(m_pITransport.Get())
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_TIMEOUT);
		
		SetCurrStatus(STATUS_UNCONNECTED);
		
		//Callback to Upper layer
		if(m_bConnectOnceMade) //May be reconnection case
		{
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECT_TIMEOUT, 
				this);
		}
		else
		{
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				(QtResult)QT_ERROR_NETWORK_CONNECT_TIMEOUT, 
				NULL, 
				m_pConnConnector.Get());
		}
	}
	else if(m_wStatus == STATUS_DATA_CAN_SEND)//For Timer2, Send & Check for Keep Alive PDU
	{
		if(m_LatestRcvTicker.overtime_sec((SERVER_UNAVAIL_TIMEOUT + 10)))
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer(), elapsed = "<< m_LatestRcvTicker.elapsed_sec());
			m_Timer.Cancel();//Cancel Timer2
			
			SetCurrStatus(STATUS_UNCONNECTED);
			
			if(m_pITransport)
			{
				m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
				m_pITransport = NULL;
			}
			
			//Callback 
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, this);
		}
		else if(m_LatestRcvTicker.overtime_sec(ALIVE_DETECT_TIMEOUT))
		{
			if(m_cBaseType == CQtConnectionManager::CTYPE_UDP) //UDP link, try connect every 5 seconds, for Vista&Mac 10.05 Mar 24 2008 Victor
			{
				QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer recv nothing in " << m_LatestRcvTicker.elapsed_sec() << " seconds, need reconnect, UDP link");
				m_ReconnectTimer.Schedule(this, (long)5, 0);
			}
			else
			{
				QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnTimer recv nothing in " << m_LatestRcvTicker.elapsed_sec() << " seconds, need reconnect, TCP link");
				Reconnect();
			}
		}
		else if(m_LatestSndTicker.overtime_sec(m_dwKeepAliveInterval))
		{
			SendKeepAlive();
		}
	}
}
	
void CRlbConnTCPClient::Reconnect()
{
	QT_INFO_TRACE_THIS("CRlbConnTCPClient::Reconnect() connector = " << m_pConnConnector);
	m_Ticker.reset();
	m_bStopFlag = FALSE;

	m_bHandshake = FALSE;
	if(m_wStatus == STATUS_CONNECTED || m_wStatus == STATUS_DATA_CAN_SEND)
	{
		if(m_pITransport)//TP layer is available
		{
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT);
		}
		else
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::Reconnect(), Need Disconnect, but TP transport is not available !");
		}
	}
	
	m_HandleTimer.Cancel();
	m_nInterval = UDP_HAND_VAL;
	
	Reset4Reconnect();
	SetCurrStatus(STATUS_NEED_RECONNECT);

	if(m_bStopFlag)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::Reconnect() connector = " << m_pConnConnector << " the connection already be cancel");
		return;
	}
	//Waiting for OnConnectIndication(...) call back
	m_pConnConnector->GetTPConnector()->CancelConnect(QT_OK);

	CQtTimeValue tvTimeOut(TIMEOUT_FOR_CONNECTION);
	m_pConnConnector->GetTPConnector()->AsycConnect(this, m_pConnConnector->GetPeerAddr(), &tvTimeOut);
}

void CRlbConnTCPClient::OnRecvConnResp()
{
	m_dwDisconnTick = 0;

	m_Timer.Cancel();
	m_HandleTimer.Cancel();
	m_ReconnectTimer.Cancel();//work for UDP Mar 24 2008 Victor
	m_nInterval = UDP_HAND_VAL;
	
	if(m_bHandShakeCancelled)//do nothing
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), connection has been cancelled.");
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_OK);
		
		return;
	}

	//Get the channel number from ConnRespPDU
	CCsPduConnReqResp presp;
	presp.Decode(*m_pmbLocData);
	WORD wChannel = presp.GetConTag();
	BYTE byConnType = presp.GetConnType();

	if(byConnType != m_byConnType)//Wrong type connection
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), Wrong connection type self type = " << m_byConnType <<
			" Response type = " << byConnType);
		Disconnect((QtResult)QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE);
		
		if(m_bConnectOnceMade) 
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), OnDisconnect() to upper layer" << " times = " << m_Ticker.elapsed_sec());
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE, 
				this);
		}
		else
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), OnConnectIndication() to upper layer" << " times = " << m_Ticker.elapsed_sec());
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE, 
				NULL, 
				m_pConnConnector.Get());
		}
		
		SetCurrStatus(STATUS_UNCONNECTED);
		return;
	}

	QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), m_wChannel = " << m_wChannel << ", wChannel = " << presp.GetConTag() << " times = " << (DWORD)m_Ticker.elapsed());
	
	//Not a Reconnection case
	if(m_wLastStatus == STATUS_UNCONNECTED && m_wStatus == STATUS_CONNECTED)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), Not a Reconnection case");

		m_wChannel = wChannel;
		//Callback ITcpRlbConnSink::OnConnect(..),Indicate upper layer can send data now.
		SetCurrStatus(STATUS_DATA_CAN_SEND);
		m_pConnConnector->GetConnectorSink()->OnConnectIndication(
			(QtResult)QT_OK, 
			this, 
			m_pConnConnector.Get());

		m_bConnectOnceMade = TRUE;
	}
	else if(m_wLastStatus == STATUS_NEED_RECONNECT && m_wStatus == STATUS_CONNECTED)	//Reconnection case
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPClient::OnRecvConnResp(), Reconnection case");

		//Reconnection, need to Do ACK. 
		if(m_dwMaxBuffLen != 0 && m_SendBuf.DoReconnACK(presp.GetACK()) == -1)//Reconnection failed, wish point must be lost
		{
			if(m_pITransport.Get())
				m_pITransport->Disconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);

			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED, 
				this);
			SetCurrStatus(STATUS_UNCONNECTED);
			return;
		}
		SetCurrStatus(STATUS_DATA_CAN_SEND);
		if(m_wChannel != 0)//Handshake was once made 
		{
			QT_ASSERTE(m_wChannel == wChannel);//Must equal!
		}
		if(m_dwMaxBuffLen != 0)
			SendDataFromSendBuf();
		else if(m_pmbRecData)
		{
			m_pmbRecData->DestroyChained();
			m_pmbRecData = NULL;
		}

		if(m_pITransportSink)
			m_pITransportSink->OnSend(this);
	}
	
	//Destroy Connection Request PDU in m_pConnReqPDU
	if(m_pConnReqPDU)
	{
		m_pConnReqPDU->DestroyChained();
		m_pConnReqPDU = NULL;
	}
	m_Timer.Schedule(this, CQtTimeValue((long)m_dwKeepAliveInterval, 0));
}

QtResult CRlbConnTCPClient::GetOption(
		DWORD aCommand, 
		LPVOID aArg)
{
	QtResult rv = QT_ERROR_NOT_AVAILABLE;

	switch(aCommand)
	{
	case QT_OPT_TRANSPORT_TRAN_TYPE:

		if(m_pITransport)
		{
			DWORD dwTransType;

			rv = m_pITransport->GetOption(
				QT_OPT_TRANSPORT_TRAN_TYPE, 
				(LPVOID)&dwTransType);

			if(QT_SUCCEEDED(rv))
			{
				*(static_cast<CQtConnectionManager::CType*>(aArg)) 
				= CQtConnectionManager::CTYPE_PDU_RELIABLE | dwTransType;
			}
		}
			
		return rv;
		
	default:
		return CCsConn::GetOption(aCommand, aArg);
	}
}

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

///////////////////////////////////////////
//class CRlbConnTCPServer
///////////////////////////////////////////
DWORD CRlbConnTCPServer::m_sdwConnectionID;
CRlbConnTCPServer::CRlbConnTCPServer(DWORD dwSendBuffSize) : CCsConn(dwSendBuffSize)
{
// 	QT_INFO_TRACE_THIS("CRlbConnTCPServer::CRlbConnTCPServer, QtRlbServerLiveKeeper m_wStatus="<<m_wStatus);
	m_pSvrList = NULL;
	m_pConnAcceptor = NULL;

//	m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();

	m_cType = CQtConnectionManager::CTYPE_PDU_RELIABLE;
	m_bPDUNeedACK = dwSendBuffSize == 0 ? FALSE : TRUE;
	m_byConnType = CS_CONN_TYPE_RLB;
	m_byInstanceType = SERVER_INSTANCE;
	m_ReleaseTimer.Schedule(this,(long)SERVER_UNAVAIL_TIMEOUT, 1);
	++m_sdwConnectionID;
	QtRlbServerLiveKeeper *pKeeper = QtRlbServerLiveKeeper::Instance();
	if(pKeeper)
		pKeeper->Register(this, m_sdwConnectionID );
}

CRlbConnTCPServer::~CRlbConnTCPServer()
{
	QT_INFO_TRACE_THIS("CRlbConnTCPServer::~CRlbConnTCPServer,QtRlbServerLiveKeeper  m_wStatus="<<m_wStatus);
	TryOnDisconnIndicate();
	QtRlbServerLiveKeeper *pKeeper = QtRlbServerLiveKeeper::Instance();
	if(pKeeper)
		pKeeper->Unregister(this, m_sdwConnectionID );
}

void CRlbConnTCPServer::TryOnDisconnIndicate()
{
	m_Timer.Cancel();
	
	if(m_wStatus == STATUS_NEED_RECONNECT)//Timeouted for Reconnection coming
	{
		if(m_pITransport)
		{
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
			m_pITransport = NULL;
		}
		SetCurrStatus(STATUS_UNCONNECTED);
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(
			(QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, 
			this);
	}

	Reset();
}

void CRlbConnTCPServer::Reset()
{
	m_pSvrList = NULL;
	m_pConnAcceptor = NULL;

	//m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();

	CCsConn::Reset();
}

QtResult CRlbConnTCPServer::Disconnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CRlbConnTCPServer::Disconnect(), m_wStatus = " << m_wStatus << 
		", m_pITransport = " << m_pITransport << " sink = " << m_pITransportSink);
	CQtComAutoPtr<CRlbConnTCPServer> pTmpSvr(this);
	m_Timer.Cancel();
	m_ReleaseTimer.Cancel();
	
	QtResult result = QT_OK;

	if(m_pITransport.Get() && m_wStatus != STATUS_UNCONNECTED)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::Disconnect(), aReason = " << aReason);
		SendDisconn(aReason);
		result = m_pITransport->Disconnect(aReason);
	}
	m_pITransport = NULL;
	SetCurrStatus(STATUS_UNCONNECTED);

	m_pITransportSink = NULL;
	QT_ASSERTE(m_pSvrList);
	m_pSvrList->RemoveServer(m_wChannel);
	return result;
}

void CRlbConnTCPServer::OnConnectIndication(
											QtResult aReason,
											IQtTransport *aTrpt,
											IQtAcceptorConnectorId *aRequestId)
{
	QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnConnectIndication, aTrpt="<<aTrpt << 
		" reason = " << aReason << "request = " << aRequestId);
	QT_ASSERTE(m_pConnAcceptor->GetTPAcceptor() == aRequestId);
	QT_ASSERTE(aTrpt);

	m_pITransport = aTrpt;
	if(aTrpt)
	{
/*		int TosV;
		QtResult rt = m_pITransport->GetOption(QT_OPT_TRANSPORT_TOS, &TosV);
		QT_ASSERTE(QT_SUCCEEDED(rt));
		QT_INFO_TRACE_THIS("TOS value = " << TosV);
*/
		DWORD dwConnectType = 0;
		m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &dwConnectType);
		if(dwConnectType & CQtConnectionManager::CTYPE_UDP)
		{
			m_dwMaxBuffLen = 0;
			m_SendBuf.SetMaxBufLen(m_dwMaxBuffLen);
		}
		aTrpt->OpenWithSink(this);
	}

	SetCurrStatus(STATUS_CONNECTED);

	//Get Transport type
	DWORD dwTransType;

	m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
	m_cBaseType = dwTransType;
}

void CRlbConnTCPServer::OnDisconnect(QtResult aReason,	IQtTransport *aTrptId)
{
	if(m_pmbRecData)
	{
		m_pmbRecData->DestroyChained();
		m_pmbRecData = NULL;
	}
	QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnDisconnect(), aReason = " << aReason << 
		" aTrptId = " << aTrptId << " sink = " << m_pITransportSink << " m_wStatus="<<m_wStatus <<
		" latest rcv elapsed = " << m_LatestRcvTicker.elapsed_sec());
	if(m_pITransport != aTrptId)
	{
		QT_ERROR_TRACE_THIS("CRlbConnTCPServer::OnDisconnect(), not match m_pITransport = " << m_pITransport << ", aTrptId = " << aTrptId);
		return;
	}

	CQtComAutoPtr<CRlbConnTCPServer> pTmpSvr(this);
	m_Timer.Cancel();
	m_ReleaseTimer.Cancel();
	
	if(m_pITransport)
		m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);

	
	if(m_bNormalDisconn/* || m_dwMaxBuffLen == 0 for reliable TCP but buffer size is 0*/)//On received DisconnPDU
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnDisconnect(), Normal Disconnect, m_pITransportSink="<<m_pITransportSink);
		SetCurrStatus(STATUS_UNCONNECTED);

		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
		
		QT_ASSERTE(m_pSvrList);
		m_pSvrList->RemoveServer(m_wChannel);
		//Waiting for a short time to be Removed
		/*m_disconn_timestamp = 
			CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);*/

		return;
	}

	if(aReason == QT_ERROR_NETWORK_SOCKET_BIND_ERROR ||
		aReason == QT_ERROR_NETWORK_CONNECT_ERROR ||
		aReason == QT_ERROR_NETWORK_CONNECT_TIMEOUT ||
		aReason == QT_ERROR_NETWORK_PDU_ERROR)
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnDisconnect(), Network error Disconnect, m_pITransportSink="<<m_pITransportSink);

		SetCurrStatus(STATUS_UNCONNECTED);
		
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
		QT_ASSERTE(m_pSvrList);
		m_pSvrList->RemoveServer(m_wChannel);
		//Waiting for a period of time to be Removed
		//m_disconn_timestamp = 
		//	CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);
	}
	else //MAY caused by Reconnection
	{
		//m_ReleaseTimer.Cancel();
//		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnDisconnect(), May Reconnection Disconnect");
		if(m_wStatus == STATUS_DATA_CAN_SEND)
		{
			Reset4ReconnComing();
			QT_ASSERTE(m_pSvrList);
			//m_pSvrList->AddServer(this, FALSE);
			SetCurrStatus(STATUS_NEED_RECONNECT);
			m_ReleaseTimer.Schedule(this,(long)SERVER_UNAVAIL_TIMEOUT, 1);
			//m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();
		}
		else
		{
			SetCurrStatus(STATUS_UNCONNECTED);
			QT_ASSERTE(m_pSvrList);
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
			m_pSvrList->RemoveServer(m_wChannel);
			/*m_disconn_timestamp = 
				CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);*/
		}			
	}
}

void CRlbConnTCPServer::Reset4ReconnComing()
{
	m_dwCnt4JudgeACK = 0;
	//m_bKeepAliveOrDataRecv = FALSE;
	//m_bDataSend = FALSE;

	if(m_pmbLocData)
	{
		m_pmbLocData->DestroyChained();
		m_pmbLocData = NULL;
	}

	/*if(m_pmbLastGet)
	{
		m_pmbLastGet->DestroyChained();
		m_pmbLastGet = NULL;
	}*/

	Reset4Recv();
}

void CRlbConnTCPServer::OnTimer(CQtTimerWrapperID* aId)
{
	//Only Send Keep Alive PDU
	if(aId == &m_ReleaseTimer)
	{
		CQtComAutoPtr<CRlbConnTCPServer> myself(this);
		QT_ASSERTE(m_pSvrList);
		SetCurrStatus(STATUS_UNCONNECTED);
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(QT_ERROR_NETWORK_CONNECT_TIMEOUT, this);//Network error, Callback to Upper layer
		m_pSvrList->RemoveServer(m_wChannel);
		return;
	}

	if(m_wStatus == STATUS_DATA_CAN_SEND)
	{
		if(m_LatestRcvTicker.overtime_sec(SERVER_UNAVAIL_TIMEOUT))
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnTimer(), elapsed = "<< m_LatestRcvTicker.elapsed_sec());
			m_Timer.Cancel();//Cancel Timer2
			
			CQtComAutoPtr<CRlbConnTCPServer> myself(this);
			
			SetCurrStatus(STATUS_UNCONNECTED);
			if(m_pITransport)
			{
				m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
				m_pITransport = NULL;
			}
			//Callback 
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, this);
			SetCurrStatus(STATUS_UNCONNECTED);
			m_pITransportSink = NULL;
			m_pSvrList->RemoveServer(m_wChannel);
		}
		else if(m_cBaseType == CQtConnectionManager::CTYPE_UDP && m_LatestRcvTicker.overtime_sec(m_dwKeepAliveInterval * 3 + 5)) //not get any packets in 3 times keepalive interval, advice Client to failover, only for UDP link
		{//server not get any packets over 30 seconds, failover Mar 23 2008 Victor
			QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnTimer(), elapsed = "<< m_LatestRcvTicker.elapsed_sec() << ", lets client failover");
			if(m_pITransport)
			{
				m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_RECEIVED_NONE);
				m_pITransport = NULL;
			}
		}
		else if(m_LatestSndTicker.overtime_sec(m_dwKeepAliveInterval))
			SendKeepAlive();
		
	}
}

void CRlbConnTCPServer::OnRecvConnReq()
{

	CQtComAutoPtr<CRlbConnTCPServer>	myself(this);
	m_ReleaseTimer.Cancel();

	//Get wChannel from PDU
	CCsPduConnReqResp preq;
	preq.Decode(*m_pmbLocData);

	WORD wChannel = preq.GetConTag();
	BYTE byConnType = preq.GetConnType();
	
	if(byConnType != m_byConnType)//Wrong Connection type
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnRecvConnReq(), Wrong Connection type");
		SendConnResp();//Send back with wrong type indication
		Disconnect(QT_ERROR_FAILURE);
		return;
	}

	if(wChannel == 0)//New connection
	{
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnRecvConnReq(), wChannel = " << wChannel << " wStatus = " << m_wStatus);
		//Whole new connection comes, Callback to Upper layer
		if(m_wLastStatus == STATUS_UNCONNECTED && m_wStatus == STATUS_CONNECTED)//Not a Reconnection case
		{
			//Callback to Upper layer
			m_pConnAcceptor->GetAcceptorSink()->OnConnectIndication(
				(QtResult)QT_OK, 
				this, 
				m_pConnAcceptor.Get());
			m_wChannel = m_pSvrList->AddServer(this);
			SetCurrStatus(STATUS_DATA_CAN_SEND);
			//use one timer for all connections
			//long lKAInterval = (long)m_dwKeepAliveInterval;
			//m_Timer.Schedule(this, CQtTimeValue(lKAInterval, 0));
		}

		SendConnResp();
	}
	else if(wChannel > 0)//Reconnection
	{
		if(m_wChannel == wChannel)//Must be resend by Client, I'v already get it before
		{
			SendConnResp();
			return;
		}
		m_wChannel = wChannel; 
		
		CQtComAutoPtr<CRlbConnTCPServer> pSvrOld = (m_pSvrList->GetServer(wChannel));
		QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnRecvConnReq(), this->m_wChannel = "<< this->m_wChannel << ", wChannel = " << wChannel << " old server handle " <<  pSvrOld.Get());
		if(pSvrOld.Get() == NULL)//pSvrOld had been removed because of unavailable timeout
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnRecvConnReq(), pSvrOld had been removed because of unavailable timeout or server restarted");
			SendDisconn(QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);		
			if(m_pITransportSink)
				m_pITransportSink->OnDisconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED,
				this);			//Disconnect
			Disconnect((QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);

			SetCurrStatus(STATUS_UNCONNECTED);
			//m_disconn_timestamp = 
			//	CQtTimeValue::GetTimeOfDay() - SERVER_UNAVAIL_TIMEOUT;
			return;
		}

		//Else, SvrOld can be found
		pSvrOld->Reset4ReconnComing();
		
		//Reconnection, need to Do ACK. 
		if(m_dwMaxBuffLen != 0 && pSvrOld->m_SendBuf.DoReconnACK(preq.GetACK()) == -1)//Reconnection failed, wish point must be lost
		{
			QT_INFO_TRACE_THIS("CRlbConnTCPServer::OnRecvConnReq(), Reconnection failed, wish point must be lost");
			if(pSvrOld->GetSink())
				pSvrOld->GetSink()->OnDisconnect(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED, 
				pSvrOld.Get());
			
			pSvrOld->SetCurrStatus(STATUS_UNCONNECTED);
			m_pSvrList->RemoveServer(pSvrOld->GetChannel());	
			SendDisconn(QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);		
			Disconnect((QtResult)QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED);
			return;
		}
		pSvrOld->Attach(this);//Attach my m_pITransport to the old
	}
}

void CRlbConnTCPServer::Attach(CRlbConnTCPServer* pSvr)
{
	QT_INFO_TRACE_THIS("CRlbConnTCPServer::Attach(), this->m_wChannel = " << m_wChannel << ", pSvr->m_wChannel = " << pSvr->m_wChannel);
	m_ReleaseTimer.Cancel();
	if(m_pITransport)
	{
		m_pITransport->Disconnect(0);
		m_pITransport = NULL;
		if(m_pmbLocData)
		{
			m_pmbLocData->DestroyChained();
			m_pmbLocData = NULL;
		}
	}
	
	m_pITransport = pSvr->m_pITransport;
	pSvr->m_pITransport = NULL;
	QT_ASSERTE_RETURN_VOID(m_pITransport);
	m_pITransport->OpenWithSink(this);
	SetCurrStatus(STATUS_DATA_CAN_SEND);
	SendConnResp();
	if(m_dwMaxBuffLen > 0 )
	{
		SendDataFromSendBuf();
	}
	else if(m_pmbRecData)
	{
		m_pmbRecData->DestroyChained();
		m_pmbRecData = NULL;
	}
	
	if(m_pITransportSink)
		m_pITransportSink->OnSend(this);

	/*! for bug  avoid fail over*/
	m_LatestRcvTicker.reset();
	// one timer for all connections
// 	long lKAInterval = (long)m_dwKeepAliveInterval;
// 	m_Timer.Schedule(this, CQtTimeValue(lKAInterval, 0));
}
void CRlbConnTCPServer::OnRecvKeepAlive()
{
	CCsConn::OnRecvKeepAlive();
	if(m_LatestSndTicker.overtime_sec(m_dwKeepAliveInterval))
		CCsConn::SendKeepAlive();
}

void CRlbConnTCPServer::SetServerList(CRlbConnTCPServerList *pSvrList)
{
	m_pSvrList = pSvrList;
}

void CRlbConnTCPServer::SetConnAcceptor(CRlbConnTCPAcceptor* pConnAcceptor)
{
	m_pConnAcceptor = pConnAcceptor;
}

CQtTimeValue CRlbConnTCPServer::GetDisconnTimestamp()
{
	return m_disconn_timestamp;
}

QtResult CRlbConnTCPServer::GetOption(
		DWORD aCommand, 
		LPVOID aArg)
{
	QtResult rv = QT_ERROR_NOT_AVAILABLE;

	switch(aCommand)
	{
	case QT_OPT_TRANSPORT_TRAN_TYPE:

		if(m_pITransport)
		{
			DWORD dwTransType;
			rv = m_pITransport->GetOption(
				QT_OPT_TRANSPORT_TRAN_TYPE, 
				(LPVOID)&dwTransType);

			if(QT_SUCCEEDED(rv))
			{
				*(static_cast<CQtConnectionManager::CType*>(aArg)) 
				= CQtConnectionManager::CTYPE_PDU_RELIABLE | dwTransType;
				rv = QT_OK;
			}
		}
			
		return rv;
		
	default:
		return CCsConn::GetOption(aCommand, aArg);
	}
}

/// 
CQtMutexThreadRecursive QtRlbServerLiveKeeper::m_sMutex;
QtRlbServerLiveKeeper* QtRlbServerLiveKeeper::m_sInstance;
using namespace std;

QtRlbServerLiveKeeper * QtRlbServerLiveKeeper::Instance()
{
	if(m_sInstance)
		return m_sInstance;
	CQtMutexGuardT<CQtMutexThreadRecursive> theGuard(m_sMutex);
	if(!m_sInstance)
		m_sInstance = new QtRlbServerLiveKeeper();
	QT_ASSERTE_RETURN(m_sInstance, m_sInstance);
	return m_sInstance;

}

QtRlbServerLiveKeeper::QtRlbServerLiveKeeper()
{
	QT_INFO_TRACE_THIS("QtRlbServerLiveKeeper::QtRlbServerLiveKeeper()");
	m_Timer.Schedule(this, SERVER_CHECK_INTERVAL * 2 + 1); //11s, bigger than client keep alive interval(10s)
	m_iter = m_ConnectionsList.end();
}

QtRlbServerLiveKeeper::~QtRlbServerLiveKeeper()
{
	QT_INFO_TRACE_THIS("QtRlbServerLiveKeeper::~QtRlbServerLiveKeeper(), size = " << m_ConnectionsList.size());
	CQtMutexGuardT<CQtMutexThreadRecursive> stheGuard(m_sMutex);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	QT_ASSERTE(m_ConnectionsList.size() == 0);
	m_ConnectionsList.clear();
	
	if(m_sInstance)
	{
		delete m_sInstance;
		m_sInstance = NULL;
	}
}

QtResult QtRlbServerLiveKeeper::Register(CRlbConnTCPServer * pServerConnection, DWORD dwID)
{
	QT_ASSERTE_RETURN(pServerConnection, QT_ERROR_FAILURE );
// 	QT_INFO_TRACE_THIS("QtRlbServerLiveKeeper::Register connection= " << pServerConnection << ", dwID = " << dwID);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	m_ConnectionsList.push_front(pServerConnection);
	return QT_OK;
}

QtResult QtRlbServerLiveKeeper::Unregister( CRlbConnTCPServer  *pServerConnection, DWORD dwID)
{
// 	QT_INFO_TRACE_THIS("QtRlbServerLiveKeeper::Unregister connection= " << pServerConnection << ", dwID = " << dwID);
	QT_ASSERTE_RETURN(pServerConnection, QT_ERROR_FAILURE );
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	std::list<CRlbConnTCPServer *>::iterator iter = m_ConnectionsList.begin();
	for(; iter != m_ConnectionsList.end(); ++iter)
	{
		if(*iter == pServerConnection)
		{
			m_iter = m_ConnectionsList.erase(iter);
			return QT_OK;
		}
	}
// 	if(0 == m_ConnectionsMap.size())
// 	{
// 		delete this;
// 		m_sInstance = NULL;
// 	}
	QT_ASSERTE(FALSE);
	return QT_ERROR_FAILURE;
}
// the timer to check all connections' status
void QtRlbServerLiveKeeper::OnTimer(CQtTimerWrapperID* aId)
{
// 	QT_INFO_TRACE_THIS("QtRlbServerLiveKeeper::OnTimer(), size = " << m_ConnectionsList.size());
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	for (m_iter = m_ConnectionsList.begin(); m_iter != m_ConnectionsList.end(); ++m_iter )	
	{
		if(*m_iter)
			(*m_iter)->OnTimer(NULL); //it maybe cause the current item has been removed from the list,
		else
			QT_ASSERTE(FALSE);
	}

}
#endif
#endif
