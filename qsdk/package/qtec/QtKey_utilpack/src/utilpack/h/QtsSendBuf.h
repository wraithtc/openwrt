/*------------------------------------------------------*/
/* Reliable Connection Send Buf classes                 */
/*                                                      */
/* CsSendBuf.h                                          */
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


#if !defined CS_SEND_BUF_H  && !defined (_NEW_PROTO_TP)
#define CS_SEND_BUF_H

#include <list>
#include "QtsPdu.h"

using namespace std;

#define FULL_RELIABLE	1			//Full reliable mode
#define SEQUENCE_LIMIT	0xFFF00000	//For Seq# rewind

class QT_OS_EXPORT CCsSendBuf  
{
public:
	CCsSendBuf(DWORD dwMaxBufLen);
	virtual ~CCsSendBuf();

	void Reset();
	void SetMaxBufLen(DWORD dwMaxBufLen);
	DWORD GetMaxBufLen();
public:
	//return : data length real accepted
	DWORD AddDataPDU(CQtMessageBlock *pData, BOOL bPDUNeedACK);
	DWORD AddKeepAlivePDU();
	DWORD AddDisconnPDU(QtResult aReason);

	BYTE GetData(CQtMessageBlock* &pmb);//get data ready for sending
	void DataSentLen(DWORD dwDataLen);//How many data have been sent this time
	int DoACK(DWORD dwSeqACKed);//ACK comes
	int DoReconnACK(DWORD dwSeqACKed);//when reconnecting ACK comes
	void SetSeq4ACK(DWORD dwSeq4ACK);

	void ClearAllSent();
private:
//	int AddToList(CCsPduSendItem *pSendPDU, DWORD dwLen);
	void AddToList(CCsPduSendItem *pSendPDU);
	DWORD DropDataIfCan(DWORD dwBytes);
	void PrepareSendMB();
private:
	DWORD m_dwMaxBufLen;				//Max of buf length
	list<CCsPduSendItem*> m_listPDU;	//list of PDU packages need to be send
	CCsPduSendItem *m_pSendPDU;			//Current sending PDU
	CQtMessageBlock *m_pmb;				//mb of current sending PDU
	DWORD m_dwPDUStartSeq;				//start sequence of every PDU
	DWORD m_dwSeqACKed;					//sequence ACKed by peer
	DWORD m_dwSeq4ACK;					//sequence for ACK to peer

	//Bytes Counter of All kinds of PDUs
	DWORD m_dwRoomsUsed;
};

#endif // #ifndef CS_SEND_BUF_H


