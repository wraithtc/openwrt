//$Id: CsSendBuf.cpp,v 1.88.4.1 2010/03/29 10:14:28 jerryh Exp $

#include "QtBase.h"
#include "QtsSendBuf.h"

#if !defined (_NEW_PROTO_TP)

CCsSendBuf::CCsSendBuf(DWORD dwMaxBufLen)
{
	m_dwMaxBufLen = dwMaxBufLen;
	m_pSendPDU = NULL;		//which SendPDU current m_pSend in
	m_dwSeqACKed = 0;		//sequence ACKed by peer
	m_dwSeq4ACK = 0;		//sequence for ACK to peer

	m_dwPDUStartSeq = 1;	//sequence begin with 1

	m_pmb = NULL;

	m_dwRoomsUsed = 0;
}

CCsSendBuf::~CCsSendBuf()
{
	if(m_listPDU.size() > 0)
	{
		QT_WARNING_TRACE_THIS("CCsSendBuf::~CCsSendBuf there have some pdu's ack not arrived! count = " << m_listPDU.size());
	}
	Reset();
}

void CCsSendBuf::Reset()
{
	//Empty the PDU list
	for(list<CCsPduSendItem*>::iterator it = m_listPDU.begin(); it != m_listPDU.end(); it++)
	{
		delete *it;
	}
	m_listPDU.clear();
	if(m_pmb)
	{
		m_pmb->DestroyChained();
		m_pmb = NULL;
	}

	m_pSendPDU = NULL;		
	m_dwSeqACKed = 0;		
	m_dwSeq4ACK = 0;		

	m_dwPDUStartSeq = 1;	

	m_dwRoomsUsed = 0;
}

DWORD CCsSendBuf::GetMaxBufLen()
{
	return m_dwMaxBufLen;
}

void CCsSendBuf::SetMaxBufLen(DWORD dwMaxBufLen)
{
//	QT_ASSERTE(dwMaxBufLen > 0);

	m_dwMaxBufLen = dwMaxBufLen;
	if(m_dwMaxBufLen == 0)
		Reset();
}

void CCsSendBuf::SetSeq4ACK(DWORD dwSeq4ACK)
{
	m_dwSeq4ACK = dwSeq4ACK;
}

void CCsSendBuf::AddToList(CCsPduSendItem *pSendPDU)
{
	QT_ASSERTE_RETURN_VOID(pSendPDU);
	if(!m_listPDU.empty())//means this SendPDU is the first member of the list
	{
		m_listPDU.back()->SetNext(pSendPDU);//let the last SendPDU in the list point to me
	}
	else
	{	
		QT_ASSERTE(!m_pSendPDU);
	}
	if(!m_pSendPDU)
	{
//		QT_INFO_TRACE_THIS("CCsSendBuf::AddToList, list size="<<m_listPDU.size());
		m_pSendPDU = pSendPDU;
		PrepareSendMB();
	}

	//push me in
	m_listPDU.push_back(pSendPDU);
	m_dwRoomsUsed += (pSendPDU->GetFixLength() + pSendPDU->GetDataLen());
}

DWORD CCsSendBuf::AddKeepAlivePDU()
{
	if(m_pSendPDU)
		return 0;
	BYTE byType = CS_PDU_TYPE_KEEPALIVE;

	if( m_dwMaxBufLen < m_dwRoomsUsed 
		|| (m_dwMaxBufLen - m_dwRoomsUsed) < CCsPduBase::GetFixLength(byType)) //no space available
	{
		QT_WARNING_TRACE_THIS("CCsSendBuf::AddKeepAlivePDU(), SendBuf is full, Keep Alive cannot be add.");
		return 0;//no space available
	}

	//Create a new Keep Alive PDU
	CCsPduKeepAlive *pSendPDU = new CCsPduKeepAlive(m_dwSeq4ACK, byType);

	////Add this PDU to list
	AddToList(pSendPDU);

	return pSendPDU->GetFixLength();
}

DWORD CCsSendBuf::AddDisconnPDU(QtResult aReason)
{
	//Create a new Keep Alive PDU
	CCsPduDisconn *pSendPDU = new CCsPduDisconn((WORD)aReason, CS_PDU_TYPE_DISCONN);

	////Add this PDU to list
	AddToList(pSendPDU);

	return pSendPDU->GetFixLength();
}

DWORD CCsSendBuf::AddDataPDU(CQtMessageBlock *pData, BOOL bPDUNeedACK)
{
	BYTE byType = CS_PDU_TYPE_DATA_NORMAL;
	DWORD dwDataLen = pData->GetChainedLength();
	
//	QT_INFO_TRACE_THIS("CCsSendBuf::AddDataPDU(), dwDataLen = " << dwDataLen);
	
	DWORD dwWholePDULen = dwDataLen + CCsPduBase::GetFixLength(byType, bPDUNeedACK);
	
	if(m_dwMaxBufLen < m_dwRoomsUsed 
		|| (m_dwMaxBufLen - m_dwRoomsUsed) < dwWholePDULen) //no space available
	{
		QT_INFO_TRACE_THIS("CCsSendBuf::AddDataPDU(), no space available");
		return 0;
	}
	
	//how many seq# can use before reach the limit
	if(SEQUENCE_LIMIT - m_dwPDUStartSeq < dwWholePDULen)
	{
		m_dwPDUStartSeq = 1;	//reset seq# to 1
		byType = CS_PDU_TYPE_DATA_START;
	}

	CQtMessageBlock *pDataTmp = pData->DuplicateChained();
	pData->AdvanceChainedReadPtr(dwDataLen);//Advance upper layer data read ptr
	//Create a new Data PDU
	CCsPduData *pSendPDU = new CCsPduData(bPDUNeedACK, m_dwSeq4ACK, dwDataLen, byType, pDataTmp);

	pSendPDU->SetSeqStart(m_dwPDUStartSeq);

	////Add this PDU to list
	AddToList(pSendPDU);

	//prepare for NEXT SendPDU
	m_dwPDUStartSeq = pSendPDU->GetSeqEnd() + 1;//the Seq next SendPDU should begin with
	return dwDataLen;
}

BYTE CCsSendBuf::GetData(CQtMessageBlock* &pmb)
{
//	QT_INFO_TRACE_THIS("CCsSendBuf::GetData, m_pmb="<<m_pmb<<", m_pSendPDU="<<m_pSendPDU);
	pmb = m_pmb;
	return m_pSendPDU?m_pSendPDU->GetType():CS_PDU_TYPE_NONE;

}

void CCsSendBuf::DataSentLen(DWORD dwDataLen)
{

	QT_ASSERTE(m_pmb);
	QT_ASSERTE(m_pmb->GetChainedLength() == 0);
	m_pmb->DestroyChained();
	m_pmb = NULL;
	QT_ASSERTE_RETURN_VOID(m_pSendPDU);
	m_pSendPDU = m_pSendPDU->GetNext();
	PrepareSendMB();
//	QT_INFO_TRACE_THIS("CCsSendBuf::DataSentLen, m_pmb="<<m_pmb<<", m_pSendPDU="<<m_pSendPDU);
	
}

void CCsSendBuf::ClearAllSent()
{
	CCsPduSendItem* pSendPdu;
	list<CCsPduSendItem*>::iterator it,it_begin;
	it = it_begin = m_listPDU.begin();
	for(; it != m_listPDU.end(); it++)
	{
		pSendPdu = *it;
		if(m_pSendPDU && pSendPdu == m_pSendPDU)
			break;
		delete pSendPdu;
	}
	if(it != it_begin)
		m_listPDU.erase(it_begin, it);
}

int CCsSendBuf::DoACK(DWORD dwSeqACKed)
{
	if(m_listPDU.empty() )//Already ACKed, when Reconnecting could bring this condition
		return -2;//Nothing to do

	CCsPduSendItem *pSendPDU = NULL;
	list<CCsPduSendItem*>::iterator it_begin = m_listPDU.begin();
	list<CCsPduSendItem*>::iterator it = it_begin;

	//Flag indicate if Reconnection ACK successfully
	////If previous DropDataIfCan() successful <m_dwSeqACKed haven't Advance, but Actually PDU dropped>, 
	////this MAY cause reconnection failed.
	////When reconnect ACK, PDUs SHOULD be moved had been moved by DropDataIfCan() before,
	////SendBuf Ptr cannot backward to the wish point, data got lost, reconnection failed!
	int nReconnACK = 0;

	//ACK has been rewound, we should DoACK to PDU whose start Seq# before 1
	//ACK all PDUs before Seq# rewind
	if(dwSeqACKed < m_dwSeqACKed)
	{
		QT_INFO_TRACE_THIS("CCsSendBuf::DoACK(), Seq# may rewind, dwSeqACKed = " << dwSeqACKed << ", m_dwSeqACKed = " << m_dwSeqACKed);
		for(;it != m_listPDU.end(); it++)
		{
			pSendPDU = *it;
			if(pSendPDU->GetType() == CS_PDU_TYPE_DATA_START && pSendPDU->GetSeqStart() == 1)
				break;
			m_dwRoomsUsed -= (pSendPDU->GetFixLength() + pSendPDU->GetDataLen());
			delete pSendPDU;
		}
	}

	//All below do when (dwSeqACKed > m_dwSeqACKed)
	//find which SendPDU dwSeqACKed belong to
	CCsPduSendItem* pNext;
	for(;it != m_listPDU.end(); it++)
	{
		pSendPDU = *it;
		if(!pSendPDU)
		{
			continue;
		}
		if(pSendPDU == m_pSendPDU)
		{
			QT_INFO_TRACE_THIS("CCsSendBuf::DoACK handle the sending PDU");
			nReconnACK = 1;
			break;
		}
		if(dwSeqACKed < pSendPDU->GetSeqEnd())
		{
			if((dwSeqACKed + 1) == pSendPDU->GetSeqStart())
				nReconnACK = 1;//But if it happen to the point!
			break;
		}
		
		m_dwRoomsUsed -= (pSendPDU->GetFixLength() + pSendPDU->GetDataLen());
		if(pSendPDU->GetType() == CS_PDU_TYPE_DATA_START 
			|| pSendPDU->GetType() == CS_PDU_TYPE_DATA_NORMAL)
		{
			m_dwSeqACKed = pSendPDU->GetSeqEnd();
		}
		
		pNext = pSendPDU->GetNext();
		delete pSendPDU;
		if(pNext && pNext->GetSeqStart() == 1)
		{
			++it;
			break;
		}
	}

	if(it != it_begin)
	{
		m_listPDU.erase(it_begin, it);
		nReconnACK = 1;
	}
	if(nReconnACK == 0)//No PDU dropped & dwSeqACKed != m_dwSeqACKed
	{
		return -1;
	}

	return 0;//OK
}

int CCsSendBuf::DoReconnACK(DWORD dwSeqACKed)
{
	QT_INFO_TRACE_THIS("CCsSendBuf::DoReconnACK(), dwSeqACKed = " << dwSeqACKed);
	//Do ACK as usual
	if(DoACK(dwSeqACKed) == -1)//ACK for Reconn failed, SendBuf Ptr cannot backward to the wish point
	{
		QT_ERROR_TRACE_THIS("CCsSendBuf::DoReconnACK ack failed and return code is -1!");
		return -1;
	}
	if(m_pmb != NULL)
	{
		m_pmb->DestroyChained();
		m_pmb = NULL;
	}
	if(!m_listPDU.empty())
	{
		m_pSendPDU = m_listPDU.front();
		PrepareSendMB();
	}
	else
		m_pSendPDU = NULL;

	//As ACK do ack to a whole PDU
	//Reset the PDU sending pointer to the first PDU in the list
	return 0;//OK
}

void CCsSendBuf::PrepareSendMB()
{
	QT_ASSERTE(!m_pmb);
	//QT_ASSERTE(m_pSendPDU);
	if(!m_pSendPDU)
		return;

	m_pSendPDU->SetACK(m_dwSeq4ACK);
	CQtMessageBlock mb(m_pSendPDU->GetFixLength());
	m_pSendPDU->Encode(mb);
	m_pmb = mb.DuplicateChained();
}

#endif
