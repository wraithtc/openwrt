/*------------------------------------------------------*/
/* the package PDU definition						    */
/*                                                      */
/* CsPacketPDU.h						                */
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

#if !defined _PACKET_PDU_H && defined _NEW_PROTO_TP
#define _PACKET_PDU_H

#include "QtDefines.h"
#include "QtDebug.h"
#include "QtMessageBlock.h"
#include "QtByteStream.h"

typedef BYTE			LINK_TYPE;
///connection type
enum{
	INVALID_LINK		= 0x00,
	RLB_LINK			= 0x01,				///the base packet link type
	PKG_LINK			= 0x02,				///the packet link that cache send request data
	REC_LINK			= 0x04,				///the packet link that PDU is length PDU type
	LEN_PKG_LINK		= 0x08,				///the link base packet link and support reconnect policy
	SENDER_PKG_LINK		= 0x10,				///the link base REC_LINK and make sure no drop packets during reconnect period
	ANY_LINK			= PKG_LINK | SENDER_PKG_LINK | REC_LINK | RLB_LINK			///server to fit client link type
};

typedef		BYTE		PACKET_TYPE;

enum {
	CS_PDU_TYPE_INVALID = 0,
	CS_PDU_TYPE_CONN_REQ,
	CS_PDU_TYPE_CONN_RESP,
	CS_PDU_TYPE_DISCONN,
	CS_PDU_TYPE_KEEPALIVE,
	CS_PDU_TYPE_DATA,
	CS_PDU_TYPE_DATA_OLD_VERSION,
	CS_PDU_TYPE_RTT_REQ,
	CS_PDU_TYPE_RTT_RESP
};

///and if enable the bellow will be not compatible to old version protocol TP
#if defined(SUPPORT_SHORT_LENGTH)
#define		PDU_LENGTH_MASK				(1 << 7) ///if the mask be set, the PDU will use short to identify the data length that length less than 64K
#define		THRESHOLD_LENGTH			((1 << 16) - 1) //the threshold of length is 2^16 - 1
#else
#define		PDU_LENGTH_MASK				0
#define		THRESHOLD_LENGTH            ((1 << 32) -1)
#endif

struct TBuff
{
	enum{ DEFAULT_BUFF_SIZE = 2048 };
	enum{ REDUSE_OCCUR_TIMES = 10 };
	void *m_pBuff;
	unsigned m_dwBuffSize;			//total buff size
	unsigned m_dwCursor;			//to identifier position
	BYTE m_byTimes;
	TBuff(unsigned dwInitSize = DEFAULT_BUFF_SIZE)
		:m_dwCursor(0), m_byTimes(0)
	{
		m_pBuff = malloc(dwInitSize);
		if(!m_pBuff)
			m_dwBuffSize = 0;
		else
			m_dwBuffSize = dwInitSize;
	}
	
	TBuff(const TBuff &rBuff)
	{
		free(m_pBuff);
		m_pBuff = rBuff.m_pBuff;
		m_dwBuffSize = rBuff.m_dwBuffSize;
		m_dwCursor = rBuff.m_dwCursor;
		m_byTimes = rBuff.m_byTimes;
		TBuff *pBuff  = const_cast<TBuff *>(&rBuff);
		pBuff->m_pBuff = NULL;
		pBuff->m_dwBuffSize = 0;
		pBuff->m_dwCursor = 0;
		pBuff->m_byTimes = 0;
	}

	TBuff & operator=(const TBuff &rBuff)
	{
		free(m_pBuff);
		m_pBuff = rBuff.m_pBuff;
		m_dwBuffSize = rBuff.m_dwBuffSize;
		m_dwCursor = rBuff.m_dwCursor;
		m_byTimes = rBuff.m_byTimes;
		TBuff *pBuff  = const_cast<TBuff *>(&rBuff);
		pBuff->m_pBuff = NULL;
		pBuff->m_dwBuffSize = 0;
		pBuff->m_dwCursor = 0;
		pBuff->m_byTimes = 0;
		return *this;
	}

	~TBuff()
	{
		free(m_pBuff);
	}
	QtResult resize(unsigned dwNewLen)
	{
		QT_ASSERTE(m_dwCursor <= dwNewLen);
		if(dwNewLen > m_dwBuffSize)
		{
			void *pBuff = realloc(m_pBuff, dwNewLen);
			if(pBuff)
			{
				m_pBuff = pBuff;
				m_dwBuffSize = dwNewLen;
				return QT_OK;
			}
			QT_ERROR_TRACE_THIS("TBuff::resize out of memory");
			///out of memory
			return QT_ERROR_FAILURE;
		}
		if(((dwNewLen < DEFAULT_BUFF_SIZE && m_dwBuffSize > DEFAULT_BUFF_SIZE) 
			&& (++m_byTimes) < REDUSE_OCCUR_TIMES))
			return QT_OK;

		m_byTimes = 0;
		if(dwNewLen > DEFAULT_BUFF_SIZE || m_dwBuffSize <= DEFAULT_BUFF_SIZE){
			return QT_OK;
		}
		
		dwNewLen = DEFAULT_BUFF_SIZE;
		void *pBuff = realloc(m_pBuff, dwNewLen);
		if(!pBuff)
		{
			QT_ERROR_TRACE_THIS("TBuff::resize out of memory");
			return QT_ERROR_FAILURE;
		}
		m_pBuff = pBuff;
		m_dwBuffSize = dwNewLen;
		QT_ASSERTE(m_dwBuffSize > m_dwCursor);
		return QT_OK;
	}
};
/*
 *	all decode will skip packet type, because of that packet type already 
 *	be read in PeekType function
 */
class CLenPacketPDU
{
public:
	CLenPacketPDU(): m_pmbData(NULL)
	{}
	~CLenPacketPDU()
	{
		delete m_pmbData;
	}
	
	static int GetFixLength(PACKET_TYPE PacketType, BOOL bNeedAck = FALSE)
	{
		return sizeof(unsigned);
	}

	static QtResult PeekType(LPVOID msg, PACKET_TYPE &type)
	{
		type = CS_PDU_TYPE_DATA;
		return QT_OK;
	}

	static QtResult PeekType(CQtMessageBlock &Block, PACKET_TYPE &type)
	{
		type = CS_PDU_TYPE_DATA;
		return QT_OK;
	}

	QtResult Encode(CQtMessageBlock& mb, CQtMessageBlock &mbData, BOOL bNeedAck, unsigned dwAck)
	{
		m_dwDataLen = mbData.GetChainedLength();
		CQtByteStreamNetwork bs(mb);
		bs << m_dwDataLen;
		mb.Append(&mbData);
		if(bs.IsGood())
			return QT_OK;

		QT_ERROR_TRACE_THIS("CLenPacketPDU::Encode failed!");
		return QT_ERROR_FAILURE;
	}

	QtResult DecodeWithoutData(CQtMessageBlock& mb, BOOL bNeedAck)
	{
		CQtByteStreamNetwork bs(mb);
		bs >> m_dwDataLen;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CLenPacketPDU::DecodeWithoutData failed!");
		return QT_ERROR_FAILURE;
	}

	QtResult DecodeWithoutData(LPVOID msg, BOOL bNeedAck)
	{
#ifdef QT_SOLARIS
		memcpy((void *)&m_dwDataLen, msg, sizeof(m_dwDataLen));
		m_dwDataLen = ntohl(m_dwDataLen);
#else
		m_dwDataLen = ntohl(*((unsigned *)msg));
#endif
		return QT_OK;
	}

	QtResult DecodeData(CQtMessageBlock& mb, BOOL bNeedAck)
	{
		if(m_pmbData){
			delete m_pmbData;
		}
		
		QT_ASSERTE(mb.GetChainedLength() >= m_dwDataLen);
		
		if(QT_FAILED(m_DataBuff.resize(m_dwDataLen)))
		{
			return QT_ERROR_FAILURE;
		}
		mb.Read(m_DataBuff.m_pBuff, m_dwDataLen);
		m_pmbData = new CQtMessageBlock(m_dwDataLen, 
			(char *)m_DataBuff.m_pBuff, 
			CQtMessageBlock::DONT_DELETE, 
			m_dwDataLen);
		if(!m_pmbData)
		{
			QT_ERROR_TRACE_THIS("CLenPacketPDU::DecodeData out of memory");
			return QT_ERROR_FAILURE;
		}
		return QT_OK;
	}	

	CQtMessageBlock *GetData(){ return m_pmbData; }
	unsigned GetDataLen() { return m_dwDataLen; }
	unsigned GetAck()	{ return 0; }
	
private:
	unsigned	m_dwDataLen;
	CQtMessageBlock		*m_pmbData;
	TBuff	m_DataBuff;
};

class CPacketPDU
{
public:
	CPacketPDU(BYTE byPacketType = CS_PDU_TYPE_INVALID):m_byPacketType(byPacketType){}
	virtual ~CPacketPDU(){}
	static int GetFixLength(PACKET_TYPE PacketType, BOOL bNeedAck = FALSE)
	{
		int dwLen = sizeof(BYTE);
		switch(PacketType & (~PDU_LENGTH_MASK)) {
		case CS_PDU_TYPE_CONN_REQ:
		case CS_PDU_TYPE_CONN_RESP:
			dwLen += sizeof(WORD) + sizeof(BYTE);
			//if(bNeedAck) //modify for compatible to old version, victor, 7/5
				dwLen += sizeof(unsigned);
			break;
		case CS_PDU_TYPE_DISCONN:
			dwLen += sizeof(WORD);
			break;
		case CS_PDU_TYPE_KEEPALIVE:
		case CS_PDU_TYPE_RTT_REQ:
		case CS_PDU_TYPE_RTT_RESP:
			dwLen += sizeof(unsigned);			
			break;
		//modify for compatible to old version, victor, 7/5
//		case CS_PDU_TYPE_CONN_RESP:
//			dwLen += sizeof(WORD) 
//			if(bNeedAck) 
//				dwLen += sizeof(unsigned);
//			break;
		case CS_PDU_TYPE_DATA_OLD_VERSION: //for old version
		case CS_PDU_TYPE_DATA:
		{
			BOOL bLengthMask = PacketType & PDU_LENGTH_MASK;
			dwLen += bLengthMask ? sizeof(WORD) : sizeof(unsigned);
			if(bNeedAck)
				dwLen += sizeof(unsigned);
			break;
		}
		default:
			QT_ASSERTE_RETURN(FALSE, -1);
		}
		return dwLen;
	}
	static QtResult PeekType(LPVOID msg, PACKET_TYPE &type)
	{
		type = *((PACKET_TYPE *)msg);
		return QT_OK;
	}

protected:
	BYTE	m_byPacketType;
};

class CDataPktPDU: public CPacketPDU
{
public:
	CDataPktPDU():CPacketPDU(CS_PDU_TYPE_DATA)
		, m_dwACK(0)
		, m_dwDataLen(0)
		, m_bNeedAck(FALSE)
		, m_pmbData(NULL)
	{}

	~CDataPktPDU()
	{
		if(m_pmbData)
			m_pmbData->DestroyChained();
	}

	QtResult DecodeWithoutData(CQtMessageBlock& mb, BOOL bNeedAck)
	{
		m_bNeedAck = bNeedAck;
		m_dwACK = 0;
		CQtByteStreamNetwork bs(mb);
		bs >> m_byPacketType;
		if(bNeedAck)
			bs >> m_dwACK;
		BOOL bLengthMask = m_byPacketType & PDU_LENGTH_MASK;
		if(bLengthMask)
		{
			WORD wLength;
			bs >> (wLength);
			m_dwDataLen = wLength;
		}
		else
			bs >> m_dwDataLen;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CDataPktPDU::DecodeWithoutData failed!");
		return QT_ERROR_FAILURE;
	}

	QtResult DecodeWithoutData(LPVOID msg, BOOL bNeedAck)
	{
		m_bNeedAck = bNeedAck;
		m_dwACK = 0;
		m_byPacketType = *((BYTE *)msg);;
		msg = (LPVOID)((BYTE *)msg + sizeof(m_byPacketType));
		if(bNeedAck)
		{
#ifdef QT_SOLARIS
			memcpy((void *)&m_dwACK, msg, sizeof(m_dwACK));
			m_dwACK = ntohl(m_dwACK);
#else
			m_dwACK = ntohl(*((unsigned *)msg));
#endif
			msg = (LPVOID)((BYTE *)msg + sizeof(m_dwACK));
		}

		BOOL bLengthMask = m_byPacketType & PDU_LENGTH_MASK;
#ifdef QT_SOLARIS
		if(bLengthMask)
		{
			WORD wLength;
			memcpy((void *)&wLength, msg, sizeof(wLength));
			m_dwDataLen = ntohs(wLength);
		}
		else
		{
			memcpy((void *)&m_dwDataLen, msg, sizeof(m_dwDataLen));
			m_dwDataLen = ntohl(m_dwDataLen);
		}
#else
		if(bLengthMask)
			m_dwDataLen = ntohs(*((unsigned short*)msg));
		else
			m_dwDataLen = ntohl(*((unsigned *)msg));
#endif
		return QT_OK;
	}

	QtResult DecodeData(CQtMessageBlock& mb, BOOL bNeedAck)
	{
		if(m_pmbData)
		{
			delete m_pmbData;
		}
		
		QT_ASSERTE(mb.GetChainedLength() >= m_dwDataLen);
		
		if(QT_FAILED(m_DataBuff.resize(m_dwDataLen)))
		{
			return QT_ERROR_FAILURE;
		}
		mb.Read(m_DataBuff.m_pBuff, m_dwDataLen);
		m_pmbData = new CQtMessageBlock(m_dwDataLen, 
			(char *)m_DataBuff.m_pBuff, 
			CQtMessageBlock::DONT_DELETE, 
			m_dwDataLen);
		if(!m_pmbData)
		{
			QT_ERROR_TRACE_THIS("CLenPacketPDU::DecodeData out of memory");
			return QT_ERROR_FAILURE;
		}
		return QT_OK;
	}
	
	QtResult Encode(CQtMessageBlock& mb, CQtMessageBlock &mbData, BOOL bNeedAck, unsigned dwAck)
	{
		m_bNeedAck = bNeedAck;
		m_dwDataLen = mbData.GetChainedLength();
#if	defined(SUPPORT_SHORT_LENGTH)
		BOOL bNeedLengthMask = m_dwDataLen < THRESHOLD_LENGTH;
		m_byPacketType = CS_PDU_TYPE_DATA | ( bNeedLengthMask ? PDU_LENGTH_MASK : 0);
#endif
		CQtByteStreamNetwork bs(mb);
		bs << m_byPacketType;
		if(m_bNeedAck)
			bs << dwAck;
#if	defined(SUPPORT_SHORT_LENGTH)
		if(bNeedLengthMask)
		{
			WORD wLength = m_dwDataLen;
			bs << wLength;
		}
		else
#endif
		bs << m_dwDataLen;
		mb.Append(&mbData);
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CDataPktPDU::Encode failed!");
		return QT_ERROR_FAILURE;
	}

	unsigned GetAck() { return m_dwACK; }
	CQtMessageBlock *GetData() { return m_pmbData; }
	unsigned GetDataLen() { return m_dwDataLen; }
	
private:
	unsigned			m_dwACK;
	unsigned			m_dwDataLen;
	BOOL				m_bNeedAck;
	CQtMessageBlock		*m_pmbData;
	TBuff				m_DataBuff;
};

class CKDRPDU: public CPacketPDU
{
public:
	CKDRPDU(PACKET_TYPE byPktType, unsigned dwValue = 0):CPacketPDU(byPktType), m_dwValue(dwValue)
	{
		if(byPktType != CS_PDU_TYPE_KEEPALIVE && 
			byPktType != CS_PDU_TYPE_DISCONN &&
			byPktType != CS_PDU_TYPE_RTT_REQ &&
			byPktType != CS_PDU_TYPE_RTT_RESP)
		{
			QT_ERROR_TRACE_THIS("CKDRPDU::CKDRPDU only for keep alive, disconnect and rtt");
			QT_ASSERTE(FALSE);
		}
	}

	QtResult Decode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs >> m_byPacketType;
		bs >> m_dwValue;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CKDRPDU::Decode failed!");
		return QT_ERROR_FAILURE;
	}
	
	QtResult Decode(TBuff &rBuff)
	{
		BYTE *pBuff = (BYTE *)rBuff.m_pBuff;
		m_byPacketType = *pBuff;
		pBuff += sizeof(m_byPacketType);
		unsigned dwLen = sizeof(m_byPacketType);
#ifdef QT_SOLARIS
		memcpy((void *)&m_dwValue, pBuff, sizeof(m_dwValue));
		m_dwValue = ntohl(m_dwValue);
#else
		m_dwValue = ntohl(*(unsigned *)pBuff);
#endif
		dwLen += sizeof(m_dwValue);
		memmove(rBuff.m_pBuff, (char *)rBuff.m_pBuff + dwLen, rBuff.m_dwCursor - dwLen);
		rBuff.m_dwCursor -= dwLen;
		return QT_OK;
	}
	
		
	QtResult Encode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs << m_byPacketType;
		bs << m_dwValue;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CKDRPDU::Encode failed!");
		return QT_ERROR_FAILURE;
	}

	unsigned GetValue() { return m_dwValue; }
private:
	unsigned	m_dwValue;
};

class CReqPDU: public CPacketPDU
{
public:
	CReqPDU(){}
	CReqPDU(BYTE byType, WORD wTag = 0, BOOL bNeedACK = FALSE, unsigned dwAck = 0)
		: CPacketPDU(CS_PDU_TYPE_CONN_REQ)
		, m_byType(byType)
		, m_wTag(wTag)
		, m_dwACK(dwAck)
		, m_bNeedAck(bNeedACK)
	{}

	QtResult Decode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs >> m_byPacketType;
//		if(RLB_LINK == m_byType)
			bs >> m_dwACK;
		bs >> m_wTag;
		bs >> m_byType;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CReqPDU::Decode failed!");
		return QT_ERROR_FAILURE;
	}

	QtResult Decode(TBuff &rBuff)
	{
		BYTE *pBuff = (BYTE *)rBuff.m_pBuff;
		m_byPacketType = *pBuff;
		pBuff += sizeof(m_byPacketType);
		unsigned dwLen = sizeof(m_byPacketType);
		//if(RLB_LINK == m_byType)
		{
#ifdef QT_SOLARIS
			memcpy((void *)&m_dwACK, pBuff, sizeof(m_dwACK));
			m_dwACK = ntohl(m_dwACK);
#else
			m_dwACK = ntohl(*(unsigned *)pBuff);
#endif
			dwLen += sizeof(m_dwACK);
			pBuff += sizeof(m_dwACK);
		}
#ifdef QT_SOLARIS
		memcpy((void *)&m_wTag, pBuff, sizeof(m_wTag));
		m_wTag = ntohs(m_wTag);
#else
		m_wTag = ntohs(*(short *)pBuff);
#endif
		pBuff += sizeof(m_wTag);
		dwLen += sizeof(m_wTag);
		m_byType = *pBuff;
	//	pBuff += sizeof(m_byType);
		dwLen += sizeof(m_byType);
		memmove(rBuff.m_pBuff, (char *)rBuff.m_pBuff + dwLen, rBuff.m_dwCursor - dwLen);
		rBuff.m_dwCursor -= dwLen;
		return QT_OK;
	}
	
	QtResult Encode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs << m_byPacketType;
		bs << m_dwACK;
		bs << m_wTag;
//		if(RLB_LINK == m_byType) // 7/5 victor
		bs << m_byType;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CReqPDU::Encode failed!");
		return QT_ERROR_FAILURE;
	}

	BYTE GetType() { return m_byType; }
	WORD GetTag() {	return m_wTag; }
	unsigned GetAck() { return m_dwACK; }

protected:
	BYTE	m_byType;
	WORD	m_wTag;
	unsigned	m_dwACK;
	BOOL	m_bNeedAck;
};

//bellow modify for compatible to old version, victor, 7/5

class CRespPDU: public CReqPDU{
public:
	CRespPDU(BOOL bNeedAck) //for decode
	{
		m_byPacketType = CS_PDU_TYPE_CONN_RESP;
		m_bNeedAck = bNeedAck;
	}
	
	CRespPDU(WORD wTag, BOOL bNeedAck, BYTE byLinkType, unsigned dwAck = 0) //for encode
	{
		m_byPacketType = CS_PDU_TYPE_CONN_RESP;
		m_wTag = wTag;
		m_byType = byLinkType;
		m_bNeedAck = bNeedAck;
		m_dwACK = dwAck;
	}
};

///add for compatible old version, Victor 7/6
class CDisconnectPDU: public CPacketPDU
{
public:
	CDisconnectPDU(WORD wValue = 0):CPacketPDU(CS_PDU_TYPE_DISCONN), m_wValue(wValue)
	{}

	QtResult Decode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs >> m_byPacketType;
		bs >> m_wValue;
		if(bs.IsGood())
			return QT_OK;
		QT_ERROR_TRACE_THIS("CDisconnectPDU::Decode decode failed!");
		return QT_ERROR_FAILURE;
	}

	QtResult Decode(TBuff &rBuff)
	{
		BYTE *pBuff = (BYTE *)rBuff.m_pBuff;
		m_byPacketType = *pBuff;
		pBuff += sizeof(m_byPacketType);
		unsigned dwLen = sizeof(m_byPacketType);
#ifdef QT_SOLARIS
		memcpy((void *)&m_wValue, pBuff, sizeof(m_dwValue));
		m_wValue = ntohs(m_wValue);
#else
		m_wValue = ntohs(*(WORD *)pBuff);
#endif
		dwLen += sizeof(m_wValue);
		memmove(rBuff.m_pBuff, (char *)rBuff.m_pBuff + dwLen, rBuff.m_dwCursor - dwLen);
		rBuff.m_dwCursor -= dwLen;
		return QT_OK;
	}
	
		
	QtResult Encode(CQtMessageBlock& mb)
	{
		CQtByteStreamNetwork bs(mb);
		bs << m_byPacketType;
		bs << m_wValue;
		if(bs.IsGood())
			return QT_OK;
		
		QT_ERROR_TRACE_THIS("CKDRPDU::Encode failed!");
		return QT_ERROR_FAILURE;
	}

	WORD GetValue() { return m_wValue; }
private:
	WORD	m_wValue;
};

#endif

