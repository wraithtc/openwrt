//$Id: CsPkg.cpp,v 1.93.4.2 2010/05/26 08:56:38 jerryh Exp $
#include "QtBase.h"
#include "QtsPkg.h"
#if !defined (_NEW_PROTO_TP)

///////////////////////////////////////////
//class CPkgConn
///////////////////////////////////////////
CPkgConn::CPkgConn(DWORD dwMaxSendBufLen) 
	: CCsConn(dwMaxSendBufLen)
	, m_dwServerUnavailTimeout(SERVER_UNAVAIL_TIMEOUT)
{
	m_cType = CQtConnectionManager::CTYPE_PDU_PACKAGE;
	m_byConnType = CS_CONN_TYPE_PKG;

	m_bPkgNeedBuf = FALSE;	//Default no SendBuf for PKG

	m_pmbLocSendData = NULL;

	m_nUngetDataCnt = 0;

	m_bNoRoom = FALSE;

	m_bConnIndicate2Upper = FALSE;
}

CPkgConn::~CPkgConn()
{
	m_Timer.Cancel();
	Reset();
}

void CPkgConn::Reset()
{
	if(m_pmbLocSendData)
	{
		m_pmbLocSendData->DestroyChained();
		m_pmbLocSendData = NULL;
	}
	CCsConn::Reset();
}

QtResult CPkgConn::SetOption(
		DWORD aCommand, 
		LPVOID aArg)
{
	switch(aCommand) 
	{
	case CS_OPT_NEED_KEEPALIVE:
		m_bNeedKeepAlive = *static_cast<BOOL*>(aArg);
		return QT_OK;
	case CS_OPT_PKG_NEED_BUF:
		m_bPkgNeedBuf = *static_cast<BOOL*>(aArg);
		return QT_OK;
	case CS_OPT_SERVER_UNAVAIL_TIMEOUT:
		m_dwServerUnavailTimeout = *static_cast<DWORD*>(aArg);
		return QT_OK;
	default:
		return CCsConn::SetOption(aCommand, aArg);
	}
}

QtResult CPkgConn::GetOption(
		DWORD aCommand, 
		LPVOID aArg)
{
	QtResult rv = QT_ERROR_NOT_AVAILABLE;
	switch(aCommand)
	{
	case CS_OPT_NEED_KEEPALIVE:
		*(static_cast<BOOL*>(aArg)) = m_bNeedKeepAlive;
		return QT_OK;
		
	case CS_OPT_PKG_NEED_BUF:
		*(static_cast<BOOL*>(aArg)) = m_bPkgNeedBuf;
		return QT_OK;

	case QT_OPT_TRANSPORT_TRAN_TYPE:
		
		if(m_pITransport)
		{
			DWORD dwTransType;

			rv = m_pITransport->GetOption(
				QT_OPT_TRANSPORT_TRAN_TYPE, 
				(LPVOID)&dwTransType);

			if(QT_SUCCEEDED(rv))
			{
				if(m_bNeedKeepAlive)
					dwTransType |= CQtConnectionManager::CTYPE_PDU_KEEPALIVE;

				*(static_cast<CQtConnectionManager::CType*>(aArg)) 
				= CQtConnectionManager::CTYPE_PDU_PACKAGE | dwTransType;

				rv = QT_OK;
			}
		}
			
		return rv;
		
	default:
		return CCsConn::GetOption(aCommand, aArg);
	}
}

QtResult CPkgConn::SendDisconn(QtResult aReason)
{
	if(m_bPkgNeedBuf)
		return CCsConn::SendDisconn(aReason);

	if(m_wStatus != STATUS_DATA_CAN_SEND)
			return QT_ERROR_NOT_AVAILABLE;//Cannot send data now

	m_SendBuf.AddDisconnPDU(aReason);	
	SendDataFromSendBuf();
	return QT_OK;
}

void CPkgConn::OnSend(IQtTransport *aTrptId, CQtTransportParameter *aPara)
{
//	QT_INFO_TRACE_THIS("CPkgConn::OnSend m_pITransport = " << m_pITransport << " aTrptId" << aTrptId);
	SetCurrStatus(STATUS_DATA_CAN_SEND);
	if(m_bPkgNeedBuf)
		CCsConn::OnSend(aTrptId, aPara);
	else
	{
		SendData_i();
		
		if(m_pITransportSink)
		{
			//QT_INFO_TRACE_THIS("CPkgSender::OnSend(), TP OnSend()");
			m_pITransportSink->OnSend(this, aPara);
		}
	}
}

QtResult CPkgConn::SendData_i()
{
	QtResult result = QT_OK;

	if(m_pmbLocSendData == NULL)
		return QT_OK;

	if(m_pmbLocSendData->GetChainedLength() > 0)//Data can send
	{
		result = m_pITransport->SendData(*m_pmbLocSendData);
	}

	return result;
}

QtResult CPkgConn::SendData(
							CQtMessageBlock &aData, 
							CQtTransportParameter *aPara)
{ 
	if(m_wStatus != STATUS_DATA_CAN_SEND)
	{
		QT_ERROR_TRACE_THIS("CPkgConn::SendData status = " << m_wStatus << " m_pITransport = " << 
			m_pITransport << " sink = " << m_pITransportSink);
		return QT_ERROR_NOT_AVAILABLE;//Cannot send data now
	}

	QT_ASSERTE_RETURN(aData.GetChainedLength() > 0, QT_OK);

	if(m_bPkgNeedBuf)
			return CCsConn::SendData(aData, aPara);

	m_LatestSndTicker.reset();
	// UDP case
	if(QT_BIT_ENABLED(m_cBaseType, CQtConnectionManager::CTYPE_UDP))
	{
		CCsPduData pd(
			FALSE, 
			0, 
			aData.GetChainedLength(), 
			CS_PDU_TYPE_DATA_NORMAL, 
			NULL);
		
		CQtMessageBlock mb(pd.GetFixLength());
		pd.EncodeWithOutData(mb);
		mb.Append(&aData);
		return m_pITransport->SendData(mb, aPara);
	}
	
	// None UDP case
	QtResult result = SendData_i();
	
	//if(m_pmbLocSendData->GetChainedLength() > 0)
	//	result = QT_ERROR_PARTIAL_DATA;

	if(QT_SUCCEEDED(result))
	{
		CCsPduData pd(
			FALSE, 
			0, 
			aData.GetChainedLength(), 
			CS_PDU_TYPE_DATA_NORMAL, 
			NULL);
		
		CQtMessageBlock mb(pd.GetFixLength());
		pd.EncodeWithOutData(mb);
		mb.Append(&aData);
		
		if(m_pmbLocSendData)
		{
			QT_ASSERTE(m_pmbLocSendData->GetChainedLength() == 0);
			m_pmbLocSendData->DestroyChained();
		}
		m_pmbLocSendData = mb.DuplicateChained();
		
		if(aPara)
			aPara->m_dwHaveSent = aData.GetChainedLength();
		
		aData.AdvanceChainedReadPtr(aData.GetChainedLength());

		SendData_i();
	}
	return result;
}

QtResult CPkgConn::SendKeepAlive()
{
	if(m_bNeedKeepAlive)
	{
		if(m_bPkgNeedBuf)
			return CCsConn::SendKeepAlive();
		else
		{
			// budingc modify at 10/17/2004.
			if(m_pmbLocSendData == NULL || m_pmbLocSendData->GetChainedLength() == 0)
			{
				CCsPduKeepAlive KApdu;
				CQtMessageBlock mb(KApdu.GetFixLength());
				KApdu.Encode(mb);

				QtResult rv = m_pITransport->SendData(mb);
				if (QT_FAILED(rv))
				{
					/*if (QT_BIT_DISABLED(m_cBaseType, CQtConnectionManager::CTYPE_UDP)) {
						if (m_pmbLocSendData)
							m_pmbLocSendData->DestroyChained();
						m_pmbLocSendData = mb.DuplicateChained();
					}*/
					QT_WARNING_TRACE_THIS("CPkgConn::SendKeepAlive, failed!");
					return rv;
				}
			}
		}
	}

	return QT_OK;
}

QtResult CPkgConn::SendDataFromSendBuf()
{
	QtResult result = CCsConn::SendDataFromSendBuf();
	m_SendBuf.ClearAllSent();//Clear all buf have been sent

	if(m_pITransportSink && m_bNoRoom)
	{
		m_bNoRoom = FALSE;
		if(QT_BIT_DISABLED(m_cBaseType, CQtConnectionManager::CTYPE_UDP))//If TCP
		{
			if(m_pITransportSink)
				m_pITransportSink->OnSend(this);
		}
	}

	return result;
}
	
void CPkgConn::ACK2PeerIfPossiable()
{
	//Do nothing, no ACK function
	return;
}

QtResult CPkgConn::Disconnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CPkgConn::Disconnect(), aReason = " << aReason
		<< " m_wStatus=" << m_wStatus << " m_pITransport = " << m_pITransport);
	m_Timer.Cancel();

	QtResult result = QT_OK;

//	CQtComAutoPtr<CPkgConn> pTmpSvr(this);
	
	if(m_pITransport)
	{
		SendDisconn(aReason);
		result = m_pITransport->Disconnect(aReason);
		//m_pITransport = NULL;
	}

	SetCurrStatus(STATUS_UNCONNECTED);

	m_pITransportSink = NULL;
	
	return result;
}
///////////////////////////////////////////
//class CPkgConnClient
///////////////////////////////////////////
CPkgConnClient::CPkgConnClient() 
	: CPkgConn(SEND_BUF_LEN_MAX4PKG)
{
	m_nConnReqCnt = 0;
	m_bConnRespRecved = FALSE;
	m_bConnectOnceMade = FALSE;
	m_bHandShakeCancelled = FALSE;

	m_pConnConnector = NULL;

	m_Timer.Schedule(this, CQtTimeValue(INTERVAL1));
	m_byInstanceType = CLIENT_INSTANCE;
}

CPkgConnClient::~CPkgConnClient()
{
}

void CPkgConnClient::OnConnectIndication(
											QtResult aReason,
											IQtTransport *aTrpt,
											IQtAcceptorConnectorId *aRequestId)
{
	QT_ASSERTE(m_pConnConnector->GetTPConnector() == aRequestId);

	if(m_bHandShakeCancelled)
	{
		QT_WARNING_TRACE_THIS("CPkgConnClient::OnConnectIndication(), aReason = " << aReason << 
			" aTrpt = " << aTrpt << "  aRequestId = " << aRequestId <<  " connection has been cancelled.");
		if(aTrpt)
			aTrpt->Disconnect((QtResult)QT_OK);

		return;
	}
	
	m_pITransport = aTrpt;
	QT_INFO_TRACE_THIS("CPkgConnClient::OnConnectIndication(), aReason = " << aReason << 
		" aTrpt = " << aTrpt << "  aRequestId = " << aRequestId << 
		" m_bHandShakeCancelled " << m_bHandShakeCancelled << 
		" times = " << m_Ticker.elapsed_sec());
	
	if(QT_SUCCEEDED(aReason))
	{
		//Get Transport type
		DWORD dwTransType = CQtConnectionManager::CTYPE_NONE;
		if(m_pITransport)
			m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
		m_cBaseType = dwTransType;
		QT_INFO_TRACE_THIS("CPkgConnClient::OnConnectIndication() connection type = " << m_cBaseType);

		m_pITransport = aTrpt;
		SetCurrStatus(STATUS_CONNECTED);//Physical connection make

		m_pITransport->OpenWithSink(this);

		SendConnReq();
		if(m_cBaseType == CQtConnectionManager::CTYPE_UDP)
			m_HandleTimer.Schedule(this, CQtTimeValue(0, m_nInterval * 1000), 1);
	}
	else //There are errors, such as timeouted
	{
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				aReason, 
				NULL, 
				m_pConnConnector);

			m_bConnIndicate2Upper = TRUE;
	}
}

void CPkgConnClient::OnDisconnect(
									 QtResult aReason,
									 IQtTransport *aTrptId)
{
	QT_INFO_TRACE_THIS("CPkgConnClient::OnDisconnect(), aReason=" << aReason << 
		" aTrptId = " << aTrptId << " m_pITransport = " << m_pITransport << " sink = " << m_pITransportSink);
	QT_ASSERTE(m_pITransport == aTrptId);
	
	m_Timer.Cancel();
	m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
	//m_pITransport = NULL;
	
	//Callback 

	if(!m_bConnIndicate2Upper)
	{
		m_pConnConnector->GetConnectorSink()->OnConnectIndication(
			(QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, 
			NULL, 
			m_pConnConnector);
		
		m_bConnIndicate2Upper = TRUE;
	}
	else
	{
		if(m_pITransportSink && STATUS_UNCONNECTED != m_wStatus)
			m_pITransportSink->OnDisconnect(aReason, this);
	}
	SetCurrStatus(STATUS_UNCONNECTED);
	m_pITransportSink = NULL;
}

void CPkgConnClient::OnTimer(CQtTimerWrapperID* aId)
{
	if(aId == &m_HandleTimer)
	{
		m_nInterval *= 2;
		m_HandleTimer.Schedule(this, CQtTimeValue(0, m_nInterval * 1000), 1);

		SendConnReq();//Resend ConnReq
		return;
	}
	else 
	{
		QT_ASSERTE(aId == &m_Timer);
	}

	if(m_wStatus == STATUS_CONNECTED || m_wStatus == STATUS_UNCONNECTED)//For Timer1 of waiting for ConnResp 
	{
		QT_INFO_TRACE_THIS("CPkgConnClient::OnTimer(), haven't recv ConnResp PDU."
			" m_cBaseType=" << m_cBaseType << 
			" m_wStatus=" << m_wStatus);
		
		if(m_pConnReqPDU)
		{
			m_pConnReqPDU->DestroyChained();
			m_pConnReqPDU = NULL;
		}
		
		m_Timer.Cancel();//Cancel Timer1
		m_HandleTimer.Cancel();
		m_nInterval = UDP_HAND_VAL;
		
		if(m_pITransport.Get())
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_TIMEOUT);
		
		SetCurrStatus(STATUS_UNCONNECTED);
		
		//Callback to Upper layer
		m_pConnConnector->GetConnectorSink()->OnConnectIndication(
			(QtResult)QT_ERROR_NETWORK_CONNECT_TIMEOUT, 
			NULL, 
			m_pConnConnector);
		
		m_bConnIndicate2Upper = TRUE;
		
		return;
		
	}
	else if(m_wStatus == STATUS_DATA_CAN_SEND)//For Timer2, Send & Check for Keep Alive PDU
	{
		if(m_bNeedKeepAlive)
		{
			if( m_LatestRcvTicker.overtime_sec(m_dwServerUnavailTimeout))
			{
				QT_INFO_TRACE_THIS("CPkgConnClient::OnTimer(), elapsed_sec = " <<(m_LatestRcvTicker.elapsed_sec()));
				m_Timer.Cancel();//Cancel Timer2
				
				SetCurrStatus(STATUS_UNCONNECTED);
				
				m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
				
				//Callback 
				if(m_pITransportSink)
					m_pITransportSink->OnDisconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, this);
				
				m_pITransportSink = NULL;
			}
			else if(m_LatestSndTicker.overtime_sec(m_dwKeepAliveInterval))
				SendKeepAlive();

		}
	}
}

void CPkgConnClient::SetConnConnector(CPkgConnConnector* pConnConnector)
{
	m_pConnConnector = pConnConnector;
}

void CPkgConnClient::CancelHandShake()
{
	//While handshaking
	if(!m_bConnectOnceMade)
	{
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_OK);
		
		m_bHandShakeCancelled = TRUE;

		// budingc 04/28/2006,
		m_Timer.Cancel();
	}
}

void CPkgConnClient::OnRecvConnResp()
{
	if(m_bHandShakeCancelled)//do nothing
	{
		QT_WARNING_TRACE_THIS("CPkgConnClient::OnRecvConnResp(), connection has been cancelled.");
		m_Timer.Cancel();
		if(m_pITransport)
			m_pITransport->Disconnect((QtResult)QT_OK);
		
		return;
	}

	if(!m_bConnRespRecved)
	{
		m_bConnRespRecved = TRUE;
		m_Timer.Cancel();
		m_HandleTimer.Cancel();
		m_nInterval = UDP_HAND_VAL;
		
		//Get the channel number from ConnRespPDU
		CCsPduConnReqResp presp;
		presp.Decode(*m_pmbLocData);
		WORD wChannel = presp.GetConTag();
		BYTE byConnType = presp.GetConnType();

		if(byConnType != m_byConnType)//Wrong type connection
		{
			QT_INFO_TRACE_THIS("CPkgConnClient::OnRecvConnResp(), Wrong connection type");
			Disconnect((QtResult)QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE);
			
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				(QtResult)QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE, 
				NULL, 
				m_pConnConnector);

			m_bConnIndicate2Upper = TRUE;
			SetCurrStatus(STATUS_UNCONNECTED);
			
			return;
		}
		
		QT_INFO_TRACE_THIS("CPkgConnClient::OnRecvConnResp(), wChannel = " << presp.GetConTag() << " times = " << m_Ticker.elapsed_sec() << 
			" Needkeepalive = " << m_bNeedKeepAlive);
		
		m_wChannel = wChannel;
		//Callback ITcpRlbConnSink::OnConnect(..),Indicate upper layer can send data now.
		SetCurrStatus(STATUS_DATA_CAN_SEND);
		m_pConnConnector->GetConnectorSink()->OnConnectIndication(
			(QtResult)QT_OK, 
			this, 
			m_pConnConnector);

		m_bConnIndicate2Upper = TRUE;
		
		m_nConnReqCnt = 0;//Reset the counter
		//Destroy Connection Request PDU in m_pConnReqPDU
		m_pConnReqPDU->DestroyChained();
		m_pConnReqPDU = NULL;
		//Re-schedule the Timer1 switch to Timer2;
		m_Timer.Schedule(this, CQtTimeValue((long)m_dwKeepAliveInterval), 0);

		m_bConnectOnceMade = TRUE;
	}
	else
	{
		QT_INFO_TRACE_THIS("CPkgConnClient::OnRecvConnResp(), Already received RESP PDU");
		m_pmbLocData->AdvanceChainedReadPtr(
			CCsPduBase::GetFixLength(CS_PDU_TYPE_CONN_RESP));
	}
}

//Client using
void CPkgConnClient::OnRecvDisconn()
{
	//Get reason from Disconn PDU
	CCsPduDisconn pdc;
	CQtMessageBlock mb(pdc.GetFixLength());
	pdc.Decode(*m_pmbLocData);

	WORD wReason = pdc.GetReason();
	QT_INFO_TRACE_THIS("CPkgConnClient::OnRecvDisconn(), Normal disconnect, wReason = " << wReason << " m_pITransport = " << 
		m_pITransport << " sink = " << m_pITransportSink);

	QtResult aReason = (QtResult)wReason;
	
	m_Timer.Cancel();
	SetCurrStatus(STATUS_UNCONNECTED);

	//Get Transport type
	DWORD dwTransType;

	if(m_pITransport)
		m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
	
	//UDP will not get OnDisconnect(...) callback, so OnDisconnect to upper layer now
//	if(QT_BIT_ENABLED(dwTransType, CQtConnectionManager::CTYPE_UDP))
	{
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
	}

	//TCP will get OnDisconnect(..) callback later
}

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

///////////////////////////////////////////
//class CPkgConnServer
///////////////////////////////////////////
CPkgConnServer::CPkgConnServer() 
	: CPkgConn(SEND_BUF_LEN_MAX4PKG)
{
	//m_pSvrList = NULL;
	m_pConnAcceptor = NULL;

//	m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();
	m_byInstanceType = SERVER_INSTANCE;

//	m_bConnReqRecved = FALSE;
}

CPkgConnServer::~CPkgConnServer()
{
	if(m_pITransport)
		m_pITransport->Disconnect(QT_OK);

	m_Timer.Cancel();
	Reset();
}

void CPkgConnServer::Reset()
{
	//m_pSvrList = NULL;
	m_pConnAcceptor = NULL;

	//m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();

	CPkgConn::Reset();
}

void CPkgConnServer::OnDisconnect(
									 QtResult aReason,
									 IQtTransport *aTrptId)
{
//	if(m_wStatus == STATUS_UNCONNECTED)
//		return;

	QT_INFO_TRACE_THIS("CPkgConnServer::OnDisconnect(), aReason = " << aReason << 
		", aTrptId = " << aTrptId << " m_pITransport = " << m_pITransport << " m_wStatus=" << m_wStatus <<
		" latest rcv elapsed = " << m_LatestRcvTicker.elapsed_sec());
	QT_ASSERTE(m_pITransport == aTrptId);
	
//	CQtComAutoPtr<CPkgConnServer> pTmpSvr(this);

	m_Timer.Cancel();

	m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
	if(m_pITransportSink && m_wStatus == STATUS_DATA_CAN_SEND)
	{
		m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
	}
	SetCurrStatus(STATUS_UNCONNECTED);

	m_pITransportSink = NULL;
}
	
void CPkgConnServer::OnTimer(CQtTimerWrapperID* aId)
{
	if(aId == &m_ReleaseTimer)//release time;
	{
		QT_INFO_TRACE_THIS("CPkgConnServer::OnTimer, m_ReleaseTimer,"
			" ref=" << GetReference() << 
			" m_wStatus=" << m_wStatus);
		QT_ASSERTE(GetReference() == 0);
		
		CQtComAutoPtr<CPkgConnServer> pTmpPkgSvr(this);
		//m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
		SetCurrStatus(STATUS_UNCONNECTED);
		return;
	}

	//keepalive time;
	QT_ASSERTE(m_bNeedKeepAlive);
	if (m_wStatus != STATUS_DATA_CAN_SEND)
		return;
	
	if(m_LatestRcvTicker.overtime_sec(m_dwServerUnavailTimeout))
	{
		QT_INFO_TRACE_THIS("CPkgConnServer::OnTimer(), elapsed_sec = "<<(m_LatestRcvTicker.elapsed_sec()));
		m_Timer.Cancel();//Cancel Timer2
		
		SetCurrStatus(STATUS_UNCONNECTED);
		
		m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
		
		//Callback 
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR, this);
		
		m_pITransportSink = NULL;
	}
	
	else if(m_LatestSndTicker.overtime_sec(m_dwKeepAliveInterval))
		SendKeepAlive();

}

void CPkgConnServer::OnConnectIndication(
											QtResult aReason,
											IQtTransport *aTrpt,
											IQtAcceptorConnectorId *aRequestId)
{
	QT_INFO_TRACE_THIS("CPkgConnServer::OnConnectIndication aReason = " << aReason << " aTrpt = " << aTrpt << " request = " << aRequestId);
	QT_ASSERTE(m_pConnAcceptor->GetTPAcceptor() == aRequestId);

	m_ReleaseTimer.Schedule(this, CQtTimeValue(TV_FOR_RELEASE), 1);

	m_pITransport = aTrpt;

	aTrpt->OpenWithSink(this);
	
	SetCurrStatus(STATUS_CONNECTED);

	//m_bConnReqRecved = FALSE;//waiting for new connection coming

	//Get Transport type
	DWORD dwTransType = CQtConnectionManager::CTYPE_NONE;

	if(m_pITransport)
		m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);

	m_cBaseType = dwTransType;
}

CQtTimeValue CPkgConnServer::GetDisconnTimestamp()
{
	return m_disconn_timestamp;
}

void CPkgConnServer::SetServerList(CPkgConnServerList *pSvrList)
{
	//m_pSvrList = pSvrList;
}

void CPkgConnServer::SetConnAcceptor(CPkgConnAcceptor* pConnAcceptor)
{
	m_pConnAcceptor = pConnAcceptor;
}

void CPkgConnServer::OnRecvConnReq()
{
	m_ReleaseTimer.Cancel();
	//Get wChannel from PDU
	CCsPduConnReqResp preq;
	preq.Decode(*m_pmbLocData);
	BYTE byConnType = preq.GetConnType();


	if(byConnType != m_byConnType)//Wrong Connection type
	{
		QT_INFO_TRACE_THIS("CPkgConnServer::OnRecvConnReq(), Wrong Connection type="<<byConnType<<",m_byConnType="<<m_byConnType<<", m_wStatus="<<m_wStatus);
		SendConnResp();//Send back with my ConnType for wrong type indication

		Disconnect(QT_ERROR_NOT_AVAILABLE);

		return;
	}

	if(m_wStatus == STATUS_CONNECTED)//first getting ConnReq PDU
	{
		QT_INFO_TRACE_THIS("CPkgConnServer::OnRecvConnReq(), New connection coming"
			" byConnType=" << byConnType << 
			" m_cBaseType=" << m_cBaseType << " times = " << m_Ticker.elapsed_sec() << 
			" Needkeepalive = " << m_bNeedKeepAlive);

		m_pConnAcceptor->GetAcceptorSink()->OnConnectIndication(
			(QtResult)QT_OK, 
			this, 
			m_pConnAcceptor.Get());

		m_bConnIndicate2Upper = TRUE;
		SendConnResp();
		SetCurrStatus(STATUS_DATA_CAN_SEND);
		
		if(m_bNeedKeepAlive)
		{
			long lKAInterval = (long)m_dwKeepAliveInterval;
			m_Timer.Schedule(this, CQtTimeValue(lKAInterval, 0));
		}
		
		m_ReleaseTimer.Cancel();
	}
	else if(m_wStatus == STATUS_DATA_CAN_SEND)
	{
		SendConnResp();
	}
	else
	{
		QT_ERROR_TRACE_THIS("CPkgConnServer::OnRecvConnReq, WRONG status, m_wStatus="<<m_wStatus);
//		Disconnect(QT_ERROR_NOT_AVAILABLE);
		QT_ASSERTE(FALSE);
	}
}
#endif
#endif
