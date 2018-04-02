

#include "QtBase.h"
#include "QtsBase.h"

#if !defined (_NEW_PROTO_TP)

///////////////////////////////////////////
//class CCsConn
///////////////////////////////////////////
CCsConn::CCsConn(DWORD dwMaxSendBufLen) :m_SendBuf(dwMaxSendBufLen)
{
	m_nInterval = UDP_HAND_VAL;
	m_pmbLocData = NULL;	
	//m_pmbLastGet = NULL;
	m_pmbRecData = NULL;			

	m_pConnReqPDU = NULL;

	m_dwSeq4ACK = 0;
	m_dwCnt4JudgeACK = 0;

	m_bNormalDisconn = FALSE;
	m_bNoRoom = FALSE;

	m_bDisableKeepaliveFlagOrNot = FALSE;

	Reset4Recv();

	//Create a SendBuf
	//m_pSendBuf = new CCsSendBuf(dwMaxSendBufLen);

	m_dwKeepAliveInterval = (DWORD)INTERVAL2;	// Default interval
	m_dwMaxBuffLen = dwMaxSendBufLen;
	m_dwRcvBytesLatestKeepalive = 0;
}

CCsConn::~CCsConn() 
{
	Reset();
	//Destroy SendBuf
	//delete m_pSendBuf;
}

void CCsConn::NeedKeepAlive(BOOL bNeedKeepAlive)
{
	m_bNeedKeepAlive = bNeedKeepAlive;
}

void CCsConn::Reset()
{
	m_dwSeq4ACK = 0;
	m_dwCnt4JudgeACK = 0;
	m_LatestSndTicker.reset();
	m_LatestRcvTicker.reset();
	if(m_pmbRecData)
	{
		m_pmbRecData->DestroyChained();
		m_pmbRecData = NULL;
	}
	
	m_SendBuf.Reset();

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

	Reset4Recv();

	//Base class
	CConnBase::Reset();
}

void CCsConn::Reset4Recv()
{
	m_dwPDULen = CCsPduBase::GetFixLength(CS_PDU_TYPE_HEAD);//2 bytes = PDU head length		
	m_byType = 0;		
	m_dwDataLen = 0;	
}

QtResult CCsConn::SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{ 
/*
	++m_dwSendCount;
	if(m_dwMaxBuffLen > 0)
		QT_INFO_TRACE_THIS("CCsConn::SendData   command, index = " << m_dwSendCount);
*/
	if(m_wStatus == STATUS_UNCONNECTED)
	{
		if(m_dwMaxBuffLen > 0)
		{
			QT_WARNING_TRACE_THIS("CCsConn::SendData send but status is wrong, transport = " << m_pITransport << " sink = " << m_pITransportSink);
		}
		return QT_ERROR_NOT_AVAILABLE;
	}
	
	QT_ASSERTE_RETURN(aData.GetChainedLength() > 0, QT_OK);
	QtResult result = QT_OK;
	
	m_dwRcvBytesLatestKeepalive = m_dwSeq4ACK;
	if(m_dwMaxBuffLen == 0)
	{

		if(m_wStatus != STATUS_DATA_CAN_SEND || !m_pITransport) //for bug 366317, only send data when connection in STATUS_DATA_CAN_SEND status 2009/9/22 Victor
		{
			return QT_ERROR_NOT_AVAILABLE;
		}

		if(m_pmbRecData)
		{
			result = m_pITransport->SendData(*m_pmbRecData);
		}
		if(QT_SUCCEEDED(result))
		{
			DWORD dwDataLen = aData.GetChainedLength();
			CQtMessageBlock *pDataTmp = aData.DuplicateChained();
			//Create a new Data PDU
			CCsPduData SendPDU (TRUE, m_dwSeq4ACK, dwDataLen, CS_PDU_TYPE_DATA_NORMAL, pDataTmp);
			if(m_pmbRecData)
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
			CQtMessageBlock mb(SendPDU.GetFixLength());
			SendPDU.Encode(mb);
			m_pmbRecData = mb.DuplicateChained();
			result = m_pITransport->SendData(*m_pmbRecData);
			if(QT_SUCCEEDED(result))
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
			m_LatestSndTicker.reset();
			aData.AdvanceChainedReadPtr(dwDataLen);//Advance upper layer data read ptr
			if(aPara)
			{
				aPara->m_dwHaveSent = dwDataLen;
			}
			return QT_OK;
		}
		return result;
	}

	//Try to send
	QtResult sendret = QT_OK;
	if(m_wStatus == STATUS_DATA_CAN_SEND)//for bug 366317, only send data when connection in STATUS_DATA_CAN_SEND status 2009/9/22 Victor
		sendret = SendDataFromSendBuf();
	
	//QtResult result = QT_OK;
	//Put aData & m_dwSeq4ACK in SendBuf, wait for send later
	/*BOOL bPDUNeedACK = FALSE;
	if(m_cType == CQtConnectionManager::CTYPE_PDU_RELIABLE)
		bPDUNeedACK = TRUE;*/
	
	DWORD dwHaveAdd = m_SendBuf.AddDataPDU(&aData, m_bPDUNeedACK/*bPDUNeedACK*/);
	if(dwHaveAdd == 0)//There are not enough rooms for the aData
	{
		QT_INFO_TRACE_THIS("CCsConn::SendData(), not enough rooms for the aData");
		m_bNoRoom = TRUE;
		result = QT_ERROR_PARTIAL_DATA;
	}
/*
	++m_dwAddCount;
	QT_INFO_TRACE_THIS("CCon::SendData add command to buff, index = " << m_dwAddCount);
*/
	if(aPara)
	{
		aPara->m_dwHaveSent = dwHaveAdd;
	}

	//Try to send
	if(QT_SUCCEEDED(sendret) && m_wStatus == STATUS_DATA_CAN_SEND)//for bug 366317, only send data when connection in STATUS_DATA_CAN_SEND status 2009/9/22 Victor
		SendDataFromSendBuf();

	return result;
}

QtResult CCsConn::SetOption(DWORD aCommand, LPVOID aArg) 
{	
//	QT_INFO_TRACE_THIS("CCsConn::SetOption(), aCommand = " << 
//		aCommand << " m_wStatus=" << m_wStatus << " transport = " << m_pITransport);
	long lKAInterval = 0;
	switch (aCommand) 
	{
	case CS_OPT_MAX_SENDBUF_LEN:
		m_dwMaxBuffLen = *static_cast<DWORD*>(aArg);
		QT_INFO_TRACE_THIS("CCsConn::SetOption set max buff len = " << m_dwMaxBuffLen);
		/*if(m_dwMaxBuffLen != 0 && m_byConnType == CS_CONN_TYPE_RLB)
			m_bPDUNeedACK = TRUE;
		else
		{
			m_bPDUNeedACK = FALSE;
		}*/
//		if(m_pSendBuf)
			m_SendBuf.SetMaxBufLen(*static_cast<DWORD*>(aArg));
		break;
	case CS_OPT_KEEPALIVE_INTERVAL:
		m_dwKeepAliveInterval = (*static_cast<DWORD*>(aArg));
		lKAInterval = (long)m_dwKeepAliveInterval;
		// budingc, check status.
		if (m_wStatus == STATUS_DATA_CAN_SEND) {
//			m_Timer.Cancel();
			m_Timer.Schedule(this, CQtTimeValue(lKAInterval, 0));
		}
		break;
	case CS_OPT_DISABLE_RCVDATA_FLAG:
		m_bDisableKeepaliveFlagOrNot = (*static_cast<BOOL *>(aArg));
		break;;
	default:
		if(m_pITransport)
			return m_pITransport->SetOption(aCommand, aArg);
		return QT_ERROR_NOT_AVAILABLE;
	}
	return QT_OK;
}

QtResult CCsConn::GetOption(DWORD aCommand, LPVOID aArg) 
{
	QT_INFO_TRACE_THIS("CCsConn::GetOption(), aCommand = " << 
		aCommand << " m_wStatus=" << m_wStatus << " transport = " << m_pITransport);
	switch (aCommand) 
	{
	case CS_OPT_MAX_SENDBUF_LEN:
		*(static_cast<DWORD*>(aArg)) = m_SendBuf.GetMaxBufLen();
		return QT_OK;
	case CS_OPT_KEEPALIVE_INTERVAL:
		*(static_cast<DWORD*>(aArg)) = m_dwKeepAliveInterval;
		return QT_OK;
	case CS_OPT_DISABLE_RCVDATA_FLAG:
		(*static_cast<BOOL *>(aArg)) = m_bDisableKeepaliveFlagOrNot;
		return QT_OK;
	default:
		if(m_pITransport)
			return m_pITransport->GetOption(aCommand, aArg);
		return QT_ERROR_NOT_AVAILABLE;
	}
}

QtResult CCsConn::Disconnect(QtResult aReason) 
{
	QT_INFO_TRACE_THIS("CCsConn::Disconnect aReason = " << aReason << " transport = " << m_pITransport << " sink = " << 
		m_pITransportSink << " status = " << m_wStatus);
	//Leave it to child
	if(m_pmbRecData)
	{
		m_pmbRecData->DestroyChained();
		m_pmbRecData = NULL;
	}
	return QT_OK;
}

void CCsConn::OnSend(IQtTransport *aTrptId, CQtTransportParameter *aPara)
{
/*
	QT_INFO_TRACE_THIS("CCsConn::OnSend transport = " << m_pITransport << " aTrptId = " << aTrptId <<
		" sink = " << m_pITransportSink << " status = " << m_wStatus );
*/
	QT_ASSERTE(m_pITransport == aTrptId);

	if(m_wStatus != STATUS_DATA_CAN_SEND)
			return;//Cannot send data now

	//Get data from SendBuf & try to send it
	//Try to send
	if(m_dwMaxBuffLen != 0)
		SendDataFromSendBuf();
	else
	{
		if(m_pmbRecData)
		{
			QtResult result = m_pITransport->SendData(*m_pmbRecData);
			if(QT_SUCCEEDED(result))
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
		}
		if(m_pITransportSink)
			m_pITransportSink->OnSend(this, aPara);
	}
	/*
	//Callback, Indicate upper layer can send data now
	if(m_pITransportSink && m_bNoRoom)
	{
		//QT_INFO_TRACE_THIS("TP layer OnSend()");
		m_bNoRoom = FALSE;
		m_pITransportSink->OnSend(this, aPara);
	}
	*/
}

QtResult CCsConn::SendKeepAlive() 
{
	//Send Keep alive with m_dwSeq4ACK
	if(m_wStatus != STATUS_DATA_CAN_SEND)
			return QT_ERROR_NOT_AVAILABLE;//Cannot send data now

	//add protection, if the transport is invalid, return directly
	QT_ASSERTE_RETURN(m_pITransport, QT_ERROR_FAILURE);

	m_dwRcvBytesLatestKeepalive = m_dwSeq4ACK;
	QtResult ret = QT_OK;
	if(m_dwMaxBuffLen == 0)
	{
		m_LatestSndTicker.reset();
		if(m_pmbRecData)
		{
			ret = m_pITransport->SendData(*m_pmbRecData);
		}
		if(QT_SUCCEEDED(ret))
		{
			if(m_pmbRecData)
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
			else
			{
				CCsPduKeepAlive SendPDU(0, CS_PDU_TYPE_KEEPALIVE);
				CQtMessageBlock mb(SendPDU.GetFixLength());
				SendPDU.Encode(mb);
				
				if(m_pmbRecData) //if it cached part of packet, append keep alive package 7/10 2008 Victor Cui
					m_pmbRecData->Append(mb.DuplicateChained());
				else
				m_pmbRecData = mb.DuplicateChained();
				QT_ASSERTE_RETURN(m_pmbRecData, QT_ERROR_FAILURE);

				ret = m_pITransport->SendData(*m_pmbRecData);
				if(QT_SUCCEEDED(ret))
				{
					m_LatestSndTicker.reset();
					m_pmbRecData->DestroyChained();
					m_pmbRecData = NULL;
				}
			}
		}
		return ret;
	}
	//Try to send
	SendDataFromSendBuf();
	
	/*ret = */
	m_SendBuf.AddKeepAlivePDU();

/*
	if(ret != 0)
	{
		++m_dwAddCount;
		QT_INFO_TRACE_THIS("CCon::Add keep alive to buff, index = " << m_dwAddCount);
	}
*/
	//Try to send
	SendDataFromSendBuf();

	return QT_OK;
}

QtResult CCsConn::SendConnReq()
{
	if(!m_pITransport)
	{
		QT_WARNING_TRACE_THIS("CCsConn::SendConnReq() connection already has been broken");
		return QT_ERROR_FAILURE;
	}

	QtResult result = QT_ERROR_FAILURE;
	int nTryTimes = 1;//Try 3 times, then give up
	//Send Conn Request with m_dwSeq4ACK & m_wChannel
	if(m_pConnReqPDU == NULL || m_pConnReqPDU->GetChainedLength() == 0)
	{
		//QT_INFO_TRACE_THIS("CCsConn::SendConnReq(), m_pConnReqPDU == NULL, make one");
		CCsPduConnReqResp preq(
			m_dwSeq4ACK, 
			m_wChannel, 
			CS_PDU_TYPE_CONN_REQ, 
			m_byConnType);
		CQtMessageBlock mb(preq.GetFixLength());
		preq.Encode(mb);
		m_pConnReqPDU = mb.DuplicateChained();//m_pConnReqPDU will be Destroyed by OnRecvConnResp
	}

	//Keep the Conn Request PDU in m_pConnReqPDU for re-sending if need
	while(QT_FAILED(result) && nTryTimes-- > 0)
	{
		CQtMessageBlock* pTmpMb = m_pConnReqPDU->DuplicateChained();
		result = m_pITransport->SendData(*pTmpMb);
		pTmpMb->DestroyChained();
	}

	return result;
}

QtResult CCsConn::SendConnResp()
{
	//QT_INFO_TRACE_THIS("CCsConn::SendConnResp(), m_dwSeq4ACK = " << m_dwSeq4ACK);
	//Send Conn Response with m_dwSeq4ACK & m_wChannel
	if(!m_pITransport)
	{
		QT_WARNING_TRACE_THIS("CCsConn::SendConnResp() connection already  has been broken");
		return QT_ERROR_FAILURE;
	}

	CCsPduConnReqResp presp(
		m_dwSeq4ACK, 
		m_wChannel, 
		CS_PDU_TYPE_CONN_RESP, 
		m_byConnType);
	
	CQtMessageBlock mb(presp.GetFixLength());
	presp.Encode(mb);

	return m_pITransport->SendData(mb);
}

QtResult CCsConn::SendDisconn(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CCsConn::SendDisconn(),  m_pITransport = " << m_pITransport <<
		" m_wStatus = "<< m_wStatus << " max buff len = " << m_dwMaxBuffLen);
	if(!m_pITransport)
	{
		QT_WARNING_TRACE_THIS("CCsConn::SendDisconn() connection already  has been broken");
		return QT_ERROR_FAILURE;
	}
	//Send Keep alive with m_dwSeq4ACK
	/*if(m_wStatus != STATUS_DATA_CAN_SEND)
			return QT_ERROR_NOT_AVAILABLE;//Cannot send data now*/

	if(m_dwMaxBuffLen == 0)
	{
		QtResult ret = QT_OK;
		if(m_pmbRecData)
		{
			ret = m_pITransport->SendData(*m_pmbRecData);
		}
		if(QT_SUCCEEDED(ret))
		{
			CCsPduDisconn SendPDU((WORD)aReason, CS_PDU_TYPE_DISCONN);
			if(m_pmbRecData)
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
			CQtMessageBlock mb(SendPDU.GetFixLength());
			SendPDU.Encode(mb);
			if(m_pmbRecData) //if it cached part of packet, append keep alive package Victor 7/10 2008
				m_pmbRecData->Append(mb.DuplicateChained());
			else
			m_pmbRecData = mb.DuplicateChained();
			QT_ASSERTE_RETURN(m_pmbRecData, QT_ERROR_FAILURE);
			ret = m_pITransport->SendData(*m_pmbRecData);
			if(QT_OK == ret)
			{
				m_pmbRecData->DestroyChained();
				m_pmbRecData = NULL;
			}
		}
		return ret;
	}
	
	//Try to send
	SendDataFromSendBuf();
	
	m_SendBuf.AddDisconnPDU(aReason);

	//Try to send
	SendDataFromSendBuf();

	return QT_OK;
}

void CCsConn::DoACK(DWORD dwACK)
{
//	QT_ASSERTE_RETURN_VOID(m_pSendBuf);
//	QT_INFO_TRACE_THIS("CCsConn::DoACK ack = " << dwACK << " Channel ID = " << m_wChannel);
	m_SendBuf.DoACK(dwACK);//Really did ACK
	
	if(m_pITransportSink && m_bNoRoom)
	{
		m_bNoRoom = FALSE;
		if(QT_BIT_DISABLED(m_cBaseType, CQtConnectionManager::CTYPE_UDP))//If TCP
		{
			m_pITransportSink->OnSend(this);
		}
	}
}

QtResult CCsConn::SendDataFromSendBuf() 
{
	if(!m_pITransport)
		return QT_ERROR_FAILURE;
	QT_ASSERTE(m_dwMaxBuffLen > 0);

	QtResult result = QT_OK;
	//Try to send data from SendBuf
	CQtMessageBlock *pmb;
	m_SendBuf.GetData(pmb);
	while(pmb/* && pmb->GetChainedLength() > 0 && QT_SUCCEEDED(result)*/)//Data can send
	{
		QT_ASSERTE(pmb->GetChainedLength() > 0);
		//DWORD dwLenBeforeSend, dwLenAfterSend = 0;
		//dwLenBeforeSend = pmb->GetChainedLength();
		result = m_pITransport->SendData(*pmb);
		//dwLenAfterSend = pmb->GetChainedLength();
//		QT_INFO_TRACE_THIS("QtsConn::SendDataFromSendBuf send command result = " << result << " max buff = " << m_dwMaxBuffLen);

		if(QT_FAILED(result))
		{
			return result;
		}
/*
		++m_dwSuccCount;
		QT_INFO_TRACE_THIS("::SendDataFromBuff, send command, index = " << m_dwSuccCount);
*/
		m_LatestSndTicker.reset();
		m_SendBuf.DataSentLen(0);
		
		/*
		 *	it is wrong, may get wrong pdu if data send partly.
		 */
		m_SendBuf.GetData(pmb);

	}
	return result;
}

void CCsConn::OnRecvConnResp()
{
	//Empty, leave it to child
	return;
}

void CCsConn::OnRecvConnReq()
{
	//Empty, leave it to child
	return;
}

//Server using
void CCsConn::OnRecvDisconn()
{
	//Get reason from Disconn PDU
	CCsPduDisconn pdc;
	CQtMessageBlock mb(pdc.GetFixLength());
	pdc.Decode(*m_pmbLocData);

	WORD wReason = pdc.GetReason();
	QT_INFO_TRACE_THIS("CCsConn::OnRecvDisconn(), Normal disconnect, wReason = " << wReason << " m_pITransport = " << m_pITransport << " status = " << m_wStatus);

	QtResult aReason = (QtResult)wReason;
	
	//Waiting for peer do Disconnection
	m_bNormalDisconn = TRUE;

	m_Timer.Cancel();
	
	// budingc modified at 10/19/2004.
	// call OnDisconnect directly in spite of UDP or TCP.

#if 0
	SetCurrStatus(STATUS_UNCONNECTED);

	//Get Transport type
	DWORD dwTransType = CQtConnectionManager::CTYPE_NONE;

	if(m_pITransport)
		m_pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, (LPVOID)&dwTransType);
	
	//UDP will not get OnDisconnect(...) callback, so OnDisconnect to upper layer now
	if(QT_BIT_ENABLED(dwTransType, CQtConnectionManager::CTYPE_UDP))
	{
		if(m_pITransportSink)
			m_pITransportSink->OnDisconnect(aReason, this);//Network error, Callback to Upper layer
		
		//Waiting for a short time to be Removed
		m_disconn_timestamp = 
			CQtTimeValue::GetTimeOfDay() - CQtTimeValue(SERVER_NORMAL_DISCONN_TIMEOUT, 0);
	}

	//TCP will get OnDisconnect(..) callback later
#else

	OnDisconnect(aReason, m_pITransport.ParaIn());
#endif
}

void CCsConn::OnRecvKeepAlive()
{
	//Only for Reliable Conneciton
	if(m_cType == CQtConnectionManager::CTYPE_PDU_RELIABLE)
	{
#ifdef _DEBUG_DUMP_
		/////////////////////Test/////////////////////////////
		CQtMessageBlock *pTest = m_pmbLocData->DuplicateChained();
		CQtByteStreamNetwork ss(*pTest);
		/////////////////////Test/////////////////////////////
#endif		
		
		//Get ACK from PDU
		CCsPduKeepAlive pka;
		pka.Decode(*m_pmbLocData);

#ifdef _DEBUG_DUMP_		
		/////////////////////Test/////////////////////////////
		::printf("OnRecvKeepAlive : ACK = %lu\n", pka.GetACK());
		char ch = '\0';
		int n = 6;
		while(n-- > 0)
		{
			ss >> ch;                            
			::printf("%02X ", ch);
		}
		::printf("\n");
		pTest->DestroyChained();
		/////////////////////Test/////////////////////////////
#endif		
		//QT_INFO_TRACE_THIS("CCsConn::OnRecvKeepAlive(), ACK = " << pka.GetACK());
		if(m_bPDUNeedACK && m_dwMaxBuffLen != 0)
			DoACK(pka.GetACK());
	}
	else	//Not Reliable Connection Case
	{
		//Only Advance KeepAlive PDU length, no ACK function
		m_pmbLocData->AdvanceChainedReadPtr(
			CCsPduBase::GetFixLength(CS_PDU_TYPE_KEEPALIVE));
	}
	
}

int CCsConn::GetPDUType()
{
/*	BOOL bPDUNeedACK = FALSE;
	if(m_cType == CQtConnectionManager::CTYPE_PDU_RELIABLE)
		bPDUNeedACK = TRUE;*/

	//avoid duplicate to improve performance, 11/2 2007 Victor Cui
	QtResult rv = m_pmbLocData->Peek(&m_byType, sizeof(m_byType));
	QT_ASSERTE(QT_SUCCEEDED(rv));
/*
	CCsPduBase pdb;
	CQtMessageBlock *pData = m_pmbLocData->DuplicateChained();

	pdb.Decode(*pData);

	m_byType = pdb.GetType();
*/
//	QT_ASSERTE(m_dwPDULen ==  CCsPduBase::GetFixLength(CS_PDU_TYPE_HEAD) || m_dwPDULen == 0 );
	m_dwPDULen = CCsPduBase::GetFixLength(m_byType, m_bPDUNeedACK/*bPDUNeedACK*/);

	if(m_dwPDULen == 0)	// Invalid PDU type
	{
		QT_WARNING_TRACE_THIS("CCsConn::GetPDUType, wrong pdu, pdu len = 0, m_byType="<<m_byType);
//		pData->DestroyChained();
		return -1;
	}

#ifdef _DEBUG_DUMP_
	FILE* fp = fopen("recv_dump.txt", "a+");
	/////////////////////Test/////////////////////////////
	fprintf(fp, "CCsConn::GetPDUType()\n");
	CQtMessageBlock *pmb = m_pmbLocData->DuplicateChained();
	DWORD dwLen = pmb->GetChainedLength();
	char* p = new char[dwLen];
	char* p1 = p;
	pmb->Read(p, dwLen);
	while(dwLen-- > 0)
	{
		fprintf(fp, "%02X ", *p1++);
	}
	fprintf(fp, "\n");
	delete[] p;
	pmb->DestroyChained();
	fclose(fp);
	/////////////////////Test/////////////////////////////
#endif
	
//	pData->DestroyChained();

	return 0;
}

void CCsConn::ACK2PeerIfPossiable(CQtMessageBlock &aData)
{
	if(m_wStatus != STATUS_DATA_CAN_SEND )
		return;

	m_dwCnt4JudgeACK += aData.GetChainedLength();

	if((m_dwCnt4JudgeACK / BYTES_TO_ACK_BACK) > 0)
	{
		m_dwCnt4JudgeACK = m_dwSeq4ACK % BYTES_TO_ACK_BACK;

		/*if(!m_bDataSend)
		{
			//QT_INFO_TRACE_THIS("CCsConn::ACK2PeerIfPossiable(), !m_bDataSend, then send KeepAlive");
			SendKeepAlive();
		}
		else
		{
			//QT_INFO_TRACE_THIS("CCsConn::ACK2PeerIfPossiable(), m_bDataSend, no need to send KeepAlive");
			m_bDataSend = FALSE;
		}*/
	}
}

void CCsConn::OnReceive(CQtMessageBlock &aData, IQtTransport *aTrptId, CQtTransportParameter *aPara)
{
	m_LatestRcvTicker.reset();
	// buding add trace info.
	DWORD pkglen = aData.GetChainedLength();
	if(STATUS_UNCONNECTED == m_wStatus)
	{
		QT_WARNING_TRACE_THIS("CCsConn::OnReceive already disconnect, data len = " << pkglen);
		return;
	}
	QT_ASSERTE(m_pITransport == aTrptId);
	if (m_pITransport != aTrptId) {
		CQtInetAddr addrLocal1, addrPeer1;
		CQtInetAddr addrLocal2, addrPeer2;
		if (m_pITransport) {
			m_pITransport->GetOption(QT_OPT_TRANSPORT_LOCAL_ADDR, &addrLocal1);
			m_pITransport->GetOption(QT_OPT_TRANSPORT_PEER_ADDR, &addrPeer1);
		}
		if (aTrptId) {
			aTrptId->GetOption(QT_OPT_TRANSPORT_LOCAL_ADDR, &addrLocal2);
			aTrptId->GetOption(QT_OPT_TRANSPORT_PEER_ADDR, &addrPeer2);
		}
		QT_ERROR_TRACE_THIS("CCsConn::OnReceive,"
			" src_ip1=" << addrLocal1.GetIpDisplayName() << 
			" src_port1" << addrLocal1.GetPort() << 
			" dst_ip1=" << addrPeer1.GetIpDisplayName() << 
			" dst_port1" << addrPeer1.GetPort() <<
			" src_ip2=" << addrLocal2.GetIpDisplayName() << 
			" src_port2" << addrLocal2.GetPort() << 
			" dst_ip2=" << addrPeer2.GetIpDisplayName() << 
			" dst_port2" << addrPeer2.GetPort());
		aData.DestroyChained();
		QT_ASSERTE_RETURN_VOID(FALSE);
	}

	if(m_pmbLocData)
		m_pmbLocData->Append(aData.DuplicateChained());
	else
		m_pmbLocData = aData.DuplicateChained();
	
	//ACK2PeerIfPossiable(aData);

	QtResult ret = QT_OK;	
	while(m_pmbLocData && m_pmbLocData->GetChainedLength() >= m_dwPDULen)
	{
		if(m_byType == 0)//get from GetPduType;
		{
			if(GetPDUType() == -1 || m_byType == 0)
			{
/*
				CQtMessageBlock *pBlock = aData.DuplicateChained();
				CQtString str = pBlock->FlattenChained();
				char *ptr = (char *)str.c_str();
				char buff[21] = {0};
				for(int i = 0; i < 10; ++i)
					sprintf(buff + (i *2), "%02X", ptr[i]);
				printf("data = %s\n", buff);
*/
				if(m_pITransport.Get())
				{
					CQtInetAddr addrPeer3;
					m_pITransport->GetOption(QT_OPT_TRANSPORT_PEER_ADDR, &addrPeer3);
					QT_ERROR_TRACE_THIS("CCsConn::OnReceive, Invalid PDU type, Drop it. m_byType=" << m_byType<<", peer addr="<<addrPeer3.GetIpDisplayName() << " len = " << pkglen /*<< " data = " << buff*/);
				}
				else
				{
					QT_ERROR_TRACE_THIS("CCsConn::OnReceive, Invalid PDU type, Drop it. m_byType=" << m_byType);
				}
				QT_ERROR_TRACE_THIS("CCsConn::OnReceive, Invalid PDU type, get data len =" << aData.GetChainedLength() << " all len = " << (m_pmbLocData ? m_pmbLocData->GetChainedLength() : 0));
				//Modify 2006.7.25 by Victor,If is invalid PUD, disconnect it and skip reliable
				OnDisconnect(QT_ERROR_NETWORK_PDU_ERROR, m_pITransport.ParaIn());
				return;
			}
		}
		//QT_INFO_TRACE_THIS("CCsConn::OnReceive, pdu="<<m_byType<<", message length="<<m_pmbLocData->GetChainedLength());
		switch(m_byType)
		{
		case CS_PDU_TYPE_DATA_START:
		case CS_PDU_TYPE_DATA_NORMAL:
		{
			if(m_wStatus != STATUS_DATA_CAN_SEND)
			{
				ret = QT_ERROR_FAILURE;
				break;
			}
			if(m_dwDataLen == 0)
			{
				// budingc check,
				QT_ASSERTE(m_dwPDULen > 0);
				if (m_pmbLocData->GetChainedLength() < m_dwPDULen)
				{
/*
					QT_INFO_TRACE_THIS("CCsConn::OnReceive, half PDU header."
						" half_len=" << m_pmbLocData->GetChainedLength() << 
						" m_dwPDULen=" << m_dwPDULen);
*/
					break;	//Waiting for next data block's coming
				}

				CCsPduData pd(m_bPDUNeedACK);
				CQtMessageBlock *pTmp = m_pmbLocData->DuplicateChained();
				pd.DecodeWithOutData(*pTmp);
				
				m_dwDataLen = pd.GetDataLen();
				/*if (m_dwDataLen == 0) {
					QT_ASSERTE(FALSE);
					ret = QT_ERROR_INVALID_ARG;
					break;
				}*/
				//QT_INFO_TRACE_THIS("m_dwPDULen="<<m_dwPDULen<<", m_dwDataLen="<<m_dwDataLen);
				m_dwPDULen += m_dwDataLen;
				
				pTmp->DestroyChained();
			}
			
			if(m_pmbLocData->GetChainedLength() >= m_dwPDULen)
			{
				CCsPduData pd(m_bPDUNeedACK);
				pd.DecodeWithOutData(*m_pmbLocData);
				if(m_bPDUNeedACK && m_dwMaxBuffLen != 0)
					DoACK(pd.GetACK());
				///modify by victorc 10/15 2004
				if(m_pITransportSink)
				{
					CQtMessageBlock *pTmp = m_pmbLocData->Disjoint(pd.GetDataLen());
					m_pITransportSink->OnReceive(
						*m_pmbLocData, 
						this, 
						aPara
						);
					m_pmbLocData->DestroyChained();
					m_pmbLocData = pTmp;
				}
				else
				{
					QT_WARNING_TRACE_THIS("CCsConn::OnReceive, can't get the sink,discard data, len ="<<pd.GetDataLen());
					m_pmbLocData->AdvanceChainedReadPtr(pd.GetDataLen());
				}
				
				m_dwSeq4ACK += m_dwPDULen;
				///if is reliable, ack when receive over 4k bytes, 2006.8.18
				if(m_dwMaxBuffLen != 0 && 
					m_cType == CQtConnectionManager::CTYPE_PDU_RELIABLE && 
					m_dwSeq4ACK - m_dwRcvBytesLatestKeepalive >= 4 * 1024)
				{
					SendKeepAlive();
				}
				//Calculate finish
				m_SendBuf.SetSeq4ACK(m_dwSeq4ACK);
				
				Reset4Recv();
			}
			break;	
		}
		
		case CS_PDU_TYPE_CONN_REQ:
		{
			if(GetInstanceType() == SERVER_INSTANCE)
			{
				OnRecvConnReq();//Server do
				Reset4Recv();
			}
			else
				ret= QT_ERROR_FAILURE;
			break;
		}
		case CS_PDU_TYPE_CONN_RESP:
		{
			if(GetInstanceType() == CLIENT_INSTANCE)
			{
				OnRecvConnResp();//Client do
				Reset4Recv();
			}
			else
			{
				ret = QT_ERROR_FAILURE;
			}
			break;
		}
		case CS_PDU_TYPE_DISCONN:
			OnRecvDisconn();
			Reset4Recv();
			return;

		case CS_PDU_TYPE_KEEPALIVE:
			OnRecvKeepAlive();
			Reset4Recv();
			break;

		default:
			ret= QT_ERROR_FAILURE;
			break;
		}
		if(QT_FAILED(ret))
		{
/*
			CQtMessageBlock *pBlock = aData.DuplicateChained();
			CQtString str = pBlock->FlattenChained();
			char *ptr = (char *)str.c_str();
			char buff[21] = {0};
			for(int i = 0; i < 10; ++i)
				sprintf(buff + (i *2), "%02X", ptr[i]);
			printf("data = %s\n", buff);
*/
			CQtInetAddr addrPeer4;
			if(m_pITransport.Get())
				m_pITransport->GetOption(QT_OPT_TRANSPORT_PEER_ADDR, &addrPeer4);
			QT_ERROR_TRACE_THIS("CCsConn::OnReceive(), Invalid PDU type,"
				" m_byType=" << m_byType << 
				" m_wStatus=" << m_wStatus << 
				" m_dwDataLen=" << m_dwDataLen<<
				" peer addr="<<addrPeer4.GetIpDisplayName());
			OnDisconnect(QT_ERROR_NOT_AVAILABLE, m_pITransport.ParaIn());
			return;
		}
	}
	//Garbage Reclaim
	if(m_pmbLocData)
	{
		//QT_ASSERTE(m_pmbLocData->GetChainedLength() == 0);
		m_pmbLocData = m_pmbLocData->ReclaimGarbage();
		//QT_INFO_TRACE_THIS("CCsConn::OnReceive, len = "<<m_pmbLocData->GetChainedLength());
		//QT_ASSERTE(m_pmbLocData == NULL);
	}
}
#endif

