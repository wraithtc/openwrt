//$Id: CsPkgSender.cpp,v 1.87.4.1 2010/03/29 10:14:28 jerryh Exp $
#include "QtBase.h"
#include "QtsPkgSender.h"

#if !defined (_NEW_PROTO_TP)

CPkgSender::CPkgSender()
{
	m_pmbLocSendData = NULL;

	m_disconn_timestamp = CQtTimeValue::GetTimeOfDay();

	m_cType = CQtConnectionManager::CTYPE_SEND_NO_PARTIAL_DATA;

	m_pConnConnector = NULL;
}

CPkgSender::~CPkgSender()
{
	m_Timer.Cancel();
	Reset();
}

void CPkgSender::TryOnDisconnIndicate()
{
	//Only RlbTcpServer may do
	return;
}

void CPkgSender::NeedKeepAlive(BOOL bNeedKeepAlive)
{
	//Do nothing
	return;
}

void CPkgSender::SetConnConnector(CPkgSenderConnector* pConnConnector)
{
	
	m_pConnConnector = pConnConnector;
}

void CPkgSender::CancelHandShake()
{
	//Do nothing
	return;
}

void CPkgSender::Reset()
{
	if(m_pmbLocSendData)
	{
		m_pmbLocSendData->DestroyChained();
		m_pmbLocSendData = NULL;
	}
	CConnBase::Reset();
}

QtResult CPkgSender::SendData_i() 
{
	QtResult result = QT_OK;

	//DWORD dwLenBeforeSend, dwLenAfterSend = 0;
	
	if(m_pmbLocSendData == NULL)
		return QT_OK;

	if(m_pmbLocSendData->GetChainedLength() > 0)//Data can send
	{
		//dwLenBeforeSend = m_pmbLocSendData->GetChainedLength();
		result = m_pITransport->SendData(*m_pmbLocSendData);
		//dwLenAfterSend = m_pmbLocSendData->GetChainedLength();
		
		//QT_INFO_TRACE_THIS("CPkgSender::SendData_i(), Data piece sent " << dwLenBeforeSend - dwLenAfterSend << " Bytes");
	}

	return result;
}

QtResult CPkgSender::SendData(
							  CQtMessageBlock &aData, 
							  CQtTransportParameter *aPara)
{
	if(m_wStatus != STATUS_DATA_CAN_SEND)
		return QT_ERROR_NOT_AVAILABLE;//Cannot send data now

	QT_ASSERTE_RETURN(aData.GetChainedLength() > 0, QT_OK);

	QtResult result = QT_OK;
		
	if(m_pmbLocSendData->GetChainedLength() > 0)
		result = QT_ERROR_PARTIAL_DATA;
	
	if(QT_SUCCEEDED(SendData_i()))
	{
		result = QT_OK;
		
		if(m_pmbLocSendData == NULL)
		{
			m_pmbLocSendData = aData.DuplicateChained();
			
			if(aPara)
				aPara->m_dwHaveSent = aData.GetChainedLength();
			
			aData.AdvanceChainedReadPtr(aData.GetChainedLength());
		}
		else if(m_pmbLocSendData->GetChainedLength() == 0)
		{
			m_pmbLocSendData->DestroyChained();
			m_pmbLocSendData = aData.DuplicateChained();
			
			if(aPara)
				aPara->m_dwHaveSent = aData.GetChainedLength();
			
			aData.AdvanceChainedReadPtr(aData.GetChainedLength());
		}
		
		SendData_i();
	}
	
	return result;
}

QtResult CPkgSender::SetOption(
								  DWORD aCommand, 
								  LPVOID aArg)
{
	if(m_pITransport)
		return m_pITransport->SetOption(aCommand, aArg);

	return QT_ERROR_NOT_AVAILABLE;
}

QtResult CPkgSender::GetOption(
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
				= CQtConnectionManager::CTYPE_SEND_NO_PARTIAL_DATA | dwTransType;

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

QtResult CPkgSender::Disconnect(
								   QtResult aReason)
{
	return m_pITransport->Disconnect(aReason);

	m_pITransportSink = NULL;
}

void CPkgSender::OnReceive(
							  CQtMessageBlock &aData,
							  IQtTransport *aTrptId,
							  CQtTransportParameter *aPara)
{
	//Call back directly
	if(m_pITransportSink)
		m_pITransportSink->OnReceive(aData, this, aPara);
}

void CPkgSender::OnSend(
						   IQtTransport *aTrptId,
						   CQtTransportParameter *aPara)
{
	SetCurrStatus(STATUS_DATA_CAN_SEND);
	SendData_i();

	if(m_pITransportSink)
	{
		//QT_INFO_TRACE_THIS("CPkgSender::OnSend(), TP OnSend()");
		m_pITransportSink->OnSend(this, aPara);
	}
}

void CPkgSender::OnDisconnect(
								 QtResult aReason,
								 IQtTransport *aTrptId)
{
	//QT_INFO_TRACE_THIS("CPkgSender::OnDisconnect reason = " << aReason << " transport = " << aTrptId);
	SetCurrStatus(STATUS_UNCONNECTED);

	m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
	
	if(m_pITransportSink)
		m_pITransportSink->OnDisconnect(aReason, this);

	m_pITransportSink = NULL;

	//Waiting for a short time to be Removed
	m_disconn_timestamp = 
		CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);
}

void CPkgSender::OnTimer(
							CQtTimerWrapperID* aId)
{
	//Timer is no need here
	return;
}

// interface IQtAcceptorConnectorSink
void CPkgSender::OnConnectIndication(
										QtResult aReason,
										IQtTransport *aTrpt,
										IQtAcceptorConnectorId *aRequestId)
{
//	QT_INFO_TRACE_THIS("CPkgSender::OnConnectIndication(), m_pITransport = " << m_pITransport << ", aTrpt = " << aTrpt);
	
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

void CPkgSender::SetServerList(CPkgSenderServerList *pSvrList)
{
	m_pSvrList = pSvrList;
}

void CPkgSender::SetConnAcceptor(CPkgSenderAcceptor* pConnAcceptor)
{
	m_pConnAcceptor = pConnAcceptor;
}

CQtTimeValue CPkgSender::GetDisconnTimestamp()
{
	return m_disconn_timestamp;
}

#endif
