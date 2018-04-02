
#include "QtBase.h"
#include "QtsLenPkg.h"
#if !defined (_NEW_PROTO_TP)

CLenPkgConn::CLenPkgConn()
{
	m_pmbSendLocData = NULL;
	m_pmbRecvLocData = NULL;
	m_pmbLastGet = NULL;
	
	m_dwPDULen = sizeof(DWORD);//Init to be 4 bytes
	m_dwDataLen = 0;
	
	m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();
	
	m_cType = CQtConnectionManager::CTYPE_PDU_LENGTH;
	
	m_pConnConnector = NULL;
}

CLenPkgConn::~CLenPkgConn()
{
	m_Timer.Cancel();
	Reset();
}

void CLenPkgConn::Reset4Recv()
{
	m_dwPDULen = sizeof(DWORD);//Reset to be 4 bytes
	m_dwDataLen = 0;
}

void CLenPkgConn::TryOnDisconnIndicate()
{
	//Only RlbTcpServer may do
	return;
}

void CLenPkgConn::NeedKeepAlive(BOOL bNeedKeepAlive)
{
	//Do nothing
	return;
}

void CLenPkgConn::SetConnConnector(CLenPkgConnConnector* pConnConnector)
{
	
	m_pConnConnector = pConnConnector;
}

void CLenPkgConn::CancelHandShake()
{
	//Do nothing
	return;
}

void CLenPkgConn::Reset()
{
	if(m_pmbSendLocData)
	{
		m_pmbSendLocData->DestroyChained();
		m_pmbSendLocData = NULL;
	}
	CConnBase::Reset();
}

QtResult CLenPkgConn::SendData_i() 
{
	QtResult result = QT_OK;
	
	//DWORD dwLenBeforeSend, dwLenAfterSend = 0;
	
	if(m_pmbSendLocData == NULL)
		return QT_OK;
	
	if(m_pmbSendLocData->GetChainedLength() > 0)//Data can send
	{
		//dwLenBeforeSend = m_pmbSendLocData->GetChainedLength();
		result = m_pITransport->SendData(*m_pmbSendLocData);
		if (QT_FAILED(result)) {
			QT_ERROR_TRACE_THIS("CLenPkgConn::SendData_i,"
				" len=" << m_pmbSendLocData->GetChainedLength());
		}
		else {
			QT_ASSERTE(m_pmbSendLocData->GetChainedLength() == 0);
			m_pmbSendLocData->DestroyChained();
			m_pmbSendLocData = NULL;
		}
		//dwLenAfterSend = m_pmbSendLocData->GetChainedLength();
		
		//QT_INFO_TRACE_THIS("CLenPkgConn::SendData_i(), Data piece sent " << dwLenBeforeSend - dwLenAfterSend << " Bytes");
	}
	
	return result;
}

QtResult CLenPkgConn::SendData(
							   CQtMessageBlock &aData, 
							   CQtTransportParameter *aPara)
{
	QtResult result = QT_OK;
	
	if(m_wStatus != STATUS_DATA_CAN_SEND)
	{
		QT_ERROR_TRACE_THIS("CLenPkgConn::SendData QT_ERROR_NOT_AVAILABLE");
		return QT_ERROR_NOT_AVAILABLE;//Cannot send data now
	}
	
	if(m_pmbSendLocData) {
		QT_ASSERTE(m_pmbSendLocData->GetChainedLength() > 0);
		return QT_ERROR_PARTIAL_DATA;
	}

	DWORD dwDataLen = aData.GetChainedLength();
	QT_ASSERTE_RETURN(dwDataLen > 0, QT_OK);
	
	CQtMessageBlock mb(sizeof(DWORD));
	CQtByteStreamNetwork bs(mb);
	bs << dwDataLen;
	mb.Append(&aData);

	result = m_pITransport->SendData(mb);
	if (QT_FAILED(result)) {
		QT_ERROR_TRACE_THIS("CLenPkgConn::SendData,"
			" len=" << dwDataLen);
		QT_ASSERTE(!m_pmbSendLocData);
		m_pmbSendLocData = mb.DuplicateChained();
		QT_ASSERTE(m_pmbSendLocData->GetChainedLength() > 0);
	}
	
	aData.AdvanceChainedReadPtr(dwDataLen);
	return QT_OK;
	
/*	if(QT_SUCCEEDED(SendData_i()))
	{
		result = QT_OK;
		
		CQtMessageBlock mb(sizeof(DWORD));
		CQtByteStreamNetwork bs(mb);
		bs << dwDataLen;
		
		if(m_pmbSendLocData == NULL)
		{
			mb.Append(&aData);
			m_pmbSendLocData = mb.DuplicateChained();
			
			if(aPara)
				aPara->m_dwHaveSent = aData.GetChainedLength();
			
			aData.AdvanceChainedReadPtr(aData.GetChainedLength());
		}
		else if(m_pmbSendLocData->GetChainedLength() == 0)
		{
			m_pmbSendLocData->DestroyChained();
			
			mb.Append(&aData);
			m_pmbSendLocData = mb.DuplicateChained();
			
			if(aPara)
				aPara->m_dwHaveSent = aData.GetChainedLength();
			
			aData.AdvanceChainedReadPtr(aData.GetChainedLength());
		}
		
		SendData_i();
	}
	
	QT_INFO_TRACE_THIS("CLenPkgConn::SendData result = " << result << " length = " << len);
	return result;*/
}

QtResult CLenPkgConn::SetOption(
								DWORD aCommand, 
								LPVOID aArg)
{
	if(m_pITransport)
		return m_pITransport->SetOption(aCommand, aArg);
	
	return QT_ERROR_NOT_AVAILABLE;
}

QtResult CLenPkgConn::GetOption(
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
					= CQtConnectionManager::CTYPE_PDU_LENGTH | dwTransType;
				
				rv = QT_OK;
			}
		}
		
		return rv;
		
	default:
		if(m_pITransport)
			return m_pITransport->GetOption(aCommand, aArg);
		
		return QT_ERROR_NOT_AVAILABLE;
	}
}

QtResult CLenPkgConn::Disconnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CLenPkgConn::Disconnect aReason = " << aReason << 
		" m_pITransport = " << m_pITransport << " sink = " << m_pITransportSink);
	m_pITransportSink = NULL;
	return m_pITransport->Disconnect(aReason);
}

void CLenPkgConn::OnReceive(
							CQtMessageBlock &aData,
							IQtTransport *aTrptId,
							CQtTransportParameter *aPara)
{
	QT_ASSERTE(m_pITransport == aTrptId);
	
	if(m_pmbLastGet)
	{
		QT_ASSERTE(!m_pmbRecvLocData);
		m_pmbLastGet->Append(aData.DuplicateChained());//Append the new Msg block
		m_pmbRecvLocData = m_pmbLastGet;
		m_pmbLastGet = NULL;
	}
	else
	{
		if(m_pmbRecvLocData)
		{
			m_pmbRecvLocData->DestroyChained();
			m_pmbRecvLocData = NULL;
		}
		
		m_pmbRecvLocData = aData.DuplicateChained();
	}
	
	while(m_pmbRecvLocData->GetChainedLength() > 0)
	{
		if(m_pmbRecvLocData->GetChainedLength() < m_dwPDULen)
		{
			QT_ASSERTE(!m_pmbLastGet);
			m_pmbLastGet = m_pmbRecvLocData;
			m_pmbRecvLocData = NULL;
			
			break;	//Waiting for next data block's coming
		}
		
		if(m_dwDataLen == 0)
		{
			if(m_pmbRecvLocData->GetChainedLength() < sizeof(m_dwDataLen))
				break;//small than PDU length, wait for data block's coming
			CQtMessageBlock *pTmp = m_pmbRecvLocData->DuplicateChained();
			CQtByteStreamNetwork bs(*pTmp);
			bs >> m_dwDataLen;
			pTmp->DestroyChained();
			///add protect for zero length, break out from the loop, 2006.5.11 Victor
			if(m_dwDataLen == 0)
			{
				QT_ERROR_TRACE_THIS("CLenPkgConn::OnReceive PDU length is zero!!! and the connection will be abort, m_pITransport = " <<
					m_pITransport << " sink = " << m_pITransportSink);
				OnDisconnect(0, m_pITransport.Get());
				return;
			}
			else
			{
				
				m_dwPDULen += m_dwDataLen;
				continue;
			}
		}
		
		if(m_pmbRecvLocData->GetChainedLength() >= m_dwPDULen)
		{
			CQtByteStreamNetwork bs(*m_pmbRecvLocData);
			bs >> m_dwDataLen;
			
			if(m_wStatus != STATUS_UNCONNECTED)
			{
				if(m_pITransportSink)
				{
					CQtMessageBlock *pTmp = m_pmbRecvLocData->Disjoint(m_dwDataLen);
					m_pITransportSink->OnReceive(
						*m_pmbRecvLocData, 
						this, 
						aPara
						);
					
					m_pmbRecvLocData->DestroyChained();
					m_pmbRecvLocData = pTmp;
				}
				else
					m_pmbRecvLocData->AdvanceChainedReadPtr(m_dwDataLen);
			}
			else
				m_pmbRecvLocData->AdvanceChainedReadPtr(m_dwDataLen);
			
			Reset4Recv();
		}
	}
	
	//Garbage Reclaim
	if(m_pmbRecvLocData)
	{
		QT_ASSERTE(m_pmbRecvLocData->GetChainedLength() == 0);
		m_pmbRecvLocData = m_pmbRecvLocData->ReclaimGarbage();
	}
	
	if(m_pmbLastGet)
	{
		m_pmbLastGet = m_pmbLastGet->ReclaimGarbage();
	}
}

void CLenPkgConn::OnSend(
						 IQtTransport *aTrptId,
						 CQtTransportParameter *aPara)
{
//	QT_INFO_TRACE_THIS("CLenPkgConn::OnSend m_pITransport = " << m_pITransport << " aTrptId" << aTrptId);
	SetCurrStatus(STATUS_DATA_CAN_SEND);
	SendData_i();
	if(m_pITransportSink)
	{
		//QT_INFO_TRACE_THIS("CLenPkgConn::OnSend(), TP OnSend()");
		m_pITransportSink->OnSend(this, aPara);
	}
}

void CLenPkgConn::OnDisconnect(
							   QtResult aReason,
							   IQtTransport *aTrptId)
{

	QT_INFO_TRACE_THIS("CLenPkgConn::OnDisconnect aReason = " << aReason << " m_pITransport = " << m_pITransport << " aTrptId" << aTrptId);
	SetCurrStatus(STATUS_UNCONNECTED);
	
	m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
	
	if(m_pITransportSink)
		m_pITransportSink->OnDisconnect(aReason, this);
	
	m_pITransportSink = NULL;
	
	//Waiting for a short time to be Removed
	m_disconn_timestamp = 
		CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);
}

void CLenPkgConn::OnTimer(
						  CQtTimerWrapperID* aId)
{
	//Timer is no need here
	return;
}

// interface IQtAcceptorConnectorSink
void CLenPkgConn::OnConnectIndication(
									  QtResult aReason,
									  IQtTransport *aTrpt,
									  IQtAcceptorConnectorId *aRequestId)
{
	QT_INFO_TRACE_THIS("CLenPkgConn::OnConnectIndication(), m_pITransport = " << m_pITransport << 
		", aTrpt = " << aTrpt << " aRequestId = " << aRequestId);
	
	if(m_pConnConnector)//Connector case
	{
		QT_ASSERTE(m_pConnConnector->GetTPConnector() == aRequestId);
		
		if(QT_SUCCEEDED(aReason))
		{
			m_pITransport = aTrpt;
			
			m_pITransport->OpenWithSink(this);
			
			//Physical connection make, and data can be sent now
			SetCurrStatus(STATUS_DATA_CAN_SEND);
			
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				aReason, 
				this, 
				m_pConnConnector);
			
			//Get Transport type
			DWORD dwTransType = CQtConnectionManager::CTYPE_TCP;
			
			if(m_pITransport)
				m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
			
			m_cBaseType = dwTransType;
		}
		else //There are errors, such as timeouted
		{
			//Callback to Upper layer
			m_pConnConnector->GetConnectorSink()->OnConnectIndication(
				aReason, 
				NULL, 
				m_pConnConnector);
		}
		
		return;
	}
	
	if(m_pConnAcceptor.Get())//Acceptor case
	{
		QT_ASSERTE(m_pConnAcceptor->GetTPAcceptor() == aRequestId);
		
		m_pITransport = aTrpt;
		
		aTrpt->OpenWithSink(this);
		
		SetCurrStatus(STATUS_DATA_CAN_SEND);
		
		//Callback to Upper layer
		m_pConnAcceptor->GetAcceptorSink()->OnConnectIndication(
			aReason, 
			this, 
			m_pConnAcceptor.ParaIn());
		
		//Get Transport type
		DWORD dwTransType = CQtConnectionManager::CTYPE_NONE;
		
		if(m_pITransport)
			m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
		
		m_cBaseType = dwTransType;
		
		return;
	}
}

void CLenPkgConn::SetServerList(CLenPkgConnServerList *pSvrList)
{
	m_pSvrList = pSvrList;
}

void CLenPkgConn::SetConnAcceptor(CLenPkgConnAcceptor* pConnAcceptor)
{
	m_pConnAcceptor = pConnAcceptor;
}

CQtTimeValue CLenPkgConn::GetDisconnTimestamp()
{
	return m_disconn_timestamp;
}

#endif
