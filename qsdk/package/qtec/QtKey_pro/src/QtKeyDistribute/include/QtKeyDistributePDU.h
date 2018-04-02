#ifndef _QT_KEYGENPDU_AVCONFERENCE_H
#define _QT_KEYGENPDU_AVCONFERENCE_H

#include "QtManagent_defines.h"

enum
{
	PDU_KEYSYN_ENC_NULL = 0,			///ENC NULL
	PDU_KEYSYN_ENC_AES, 				///ENC AES
	PDU_KEYSYN_ENC_3DES,				///ENC 3DES
	PDU_KEYSYN_ENC_BLOWFISH 			///ENC BLOWFISH
};

class CQtSessionPDUBase
{
public:
	CQtSessionPDUBase() 
		: m_cType( QT_SESSION_PDU_TYPE_BASE )
	{} 

	virtual ~CQtSessionPDUBase() {}

public:
	QtResult Encode( CQtMessageBlock& mbBlock /* out */ )
	{
		CQtByteStreamNetwork bsStream( mbBlock );
		bsStream << m_cType;
		#if 0
		if ( !bsStream.IsGood() )
		{
			return QT_ERROR_FAILURE;
		}
		#endif
		return QT_OK;
	}

	QtResult Decode( CQtMessageBlock& mbBlock /* out */ )
	{
		CQtByteStreamNetwork bsStream( mbBlock );
		bsStream >> m_cType;
		
		if ( !bsStream.IsGood() )
		{
			return QT_ERROR_FAILURE;
		}
		

		return QT_OK;
	}

	DWORD GetLength()
	{
		return sizeof( m_cType );
	}

	QT_SESSION_TYPE GetPDUType()
	{	
		return m_cType;
	}

	static QtResult PeekType( const CQtMessageBlock& mbBlock, BYTE& cType /* out */ )
	{
		BYTE pBuf[1];
		QtResult QtResult = const_cast<CQtMessageBlock&>( mbBlock ).Read(
			pBuf, 
			sizeof(pBuf), 
			NULL, 
			FALSE );

		if ( QT_FAILED(QtResult) ) 
		{
			return QtResult;
		}

		cType = pBuf[0];

		return QT_OK;
	}

protected:
	BYTE							m_cType;
};

class CQtKeySynReq : public CQtSessionPDUBase
{
public:
	CQtKeySynReq()
		: m_wRequestID(0)
		, m_bySpecified(0)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
		, m_bShareNumber(0)
	{
		m_cType = QT_KEY_SYN_PDU_REQ;
	}

	CQtKeySynReq(WORD wRequestId,
			BYTE byKeyType,
			BYTE byKeyNum,
			BYTE byKeyLength,
			BYTE bySpecified,
			BYTE byShareNumber,
			QT_LONG LocalNodeId,
			CQtDeviceId &LocalDeviceId,
			vector<QT_LONG> &lPeerUserId,
			ListDeviceId &lPeerDeviceId)
		: m_wRequestID(wRequestId)
		, m_bySpecified(0)
		, m_byKeyType(byKeyType)
		, m_byKeyNum(byKeyNum)
		, m_byKeyLength(byKeyLength)
		, m_bShareNumber(byShareNumber)
		, m_LocalUserId(LocalNodeId)
		, m_LocalDeviceId(LocalDeviceId)
		, m_lPeerUserId(lPeerUserId)
		, m_lPeerDeviceId(lPeerDeviceId)
	{
		m_cType = QT_KEY_SYN_PDU_REQ;
	}

	virtual ~CQtKeySynReq() {}

public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byKeyType;
		bsStream << m_byKeyNum;
		bsStream << m_byKeyLength;
		bsStream << m_bySpecified;
		bsStream << m_bShareNumber;
		bsStream << m_LocalUserId;

		rv = m_LocalDeviceId.Encode(mbBlock);
		if (QT_OK != rv){
			return rv;
		}

		if(1 == m_bySpecified && 0 != m_bShareNumber)
		{
			vector<QT_LONG>::iterator it = m_lPeerUserId.begin();
			ListDeviceId::iterator it2 = m_lPeerDeviceId.begin();
			for (; it != m_lPeerUserId.end() && it2 != m_lPeerDeviceId.end(); it++ , it2++)
			{
				bsStream << *it;
				rv = it2->Encode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
			}
		}
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock /* out */)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if(QT_OK != rv)
		{
		    PRINT("CQtSessionPDUBase:Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream >> m_wRequestID;
		bsStream >> m_byKeyType;
		bsStream >> m_byKeyNum;
		bsStream >> m_byKeyLength;
		bsStream >> m_bySpecified;
		bsStream >> m_bShareNumber;
		bsStream >> m_LocalUserId;
		
		rv = m_LocalDeviceId.Decode(mbBlock);
		if (QT_OK != rv){
			return rv;
		}
		
		if(1 == m_bySpecified && 0 != m_bShareNumber)
		{
			QT_LONG UserId_tmp = 0;
			CQtDeviceId DeviceId_tmp(DEFAULT_KEY_ID_LEN);
			for (int i = 0; i < m_bShareNumber; i++)
			{
				UserId_tmp = 0;
				bsStream >> UserId_tmp;
				if (QT_OK != rv){
					return rv;
				}
				DeviceId_tmp.Clear();
				rv = DeviceId_tmp.Decode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
				m_lPeerUserId.push_back(UserId_tmp);
				m_lPeerDeviceId.push_back(DeviceId_tmp);
			}
		}

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byKeyType) +
			sizeof(m_byKeyNum) +
			sizeof(m_byKeyLength) +
			sizeof(m_bySpecified) +
			sizeof(m_bShareNumber) + 
			sizeof(m_LocalUserId);

		dwLen += DEFAULT_DEVICEID_LEN;

		if(1 == m_bySpecified && 0 != m_bShareNumber)
			dwLen += (sizeof(m_LocalUserId) + DEFAULT_DEVICEID_LEN) * m_bShareNumber;

		return dwLen;
	}
	

	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	BYTE GetKeyType()
	{
		return m_byKeyType;
	}

	BYTE GetKeyNum()
	{
		return m_byKeyNum;
	}

	BYTE GetKeyLength(){
		return m_byKeyLength;
	}

	BYTE GetSpecified(){
		return m_bySpecified;
	}

	BYTE GetShareNumber(){
		return m_bShareNumber;
	}
	
	QT_LONG GetLocalUserId(){
		return m_LocalUserId;
	}

	CQtDeviceId GetLocalDeviceId(){
		return m_LocalDeviceId;
	}

	vector<QT_LONG> GetPeerUserId(){
		return m_lPeerUserId;
	}

	ListDeviceId GetPeerDeviceId(){
		return m_lPeerDeviceId;
	}
	
protected:
	WORD m_wRequestID;
	BYTE m_byKeyType;
	BYTE m_byKeyNum;
	BYTE m_byKeyLength;
	WORD m_bySpecified;
	BYTE m_bShareNumber;
	QT_LONG 		m_LocalUserId;
	CQtDeviceId 	m_LocalDeviceId;
	vector<QT_LONG> m_lPeerUserId;
	ListDeviceId 	m_lPeerDeviceId;
};

class CQtKeySynResp : public CQtSessionPDUBase
{
public:
	CQtKeySynResp()
		: m_wRequestID(0)
		, m_byStatus(0)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
		, m_byReserved(0)
	{
		m_cType = QT_KEY_SYN_PDU_RESP;
	}

	CQtKeySynResp(WORD wRequestId, BYTE byStatus)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
		, m_byReserved(0)
	{
		m_cType = QT_KEY_SYN_PDU_RESP;	
	}

	CQtKeySynResp(WORD wRequestId,
			BYTE byStatus,
			BYTE byKeyType,
			BYTE byKeyNum,
			BYTE byKeyLength,
			QT_LONG &LocalNodeId,
			CQtDeviceId &LocalDeviceId,
			ListKeyID &KeyIdList,
			ListKey &KeyList)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
		, m_byKeyType(byKeyType)
		, m_byKeyNum(byKeyNum)
		, m_byKeyLength(byKeyLength)
		, m_byReserved(0)
		, m_LocalNodeId(LocalNodeId)
		, m_LocalDeviceId(LocalDeviceId)
		, m_KeyIdList(KeyIdList)
		, m_KeyList(KeyList)
	{
		m_cType = QT_KEY_SYN_PDU_RESP;
	}

	virtual ~CQtKeySynResp() {}
	
public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byStatus;
		bsStream << m_byKeyType;
		bsStream << m_byKeyNum;
		bsStream << m_byKeyLength;
		bsStream << m_byReserved;
		bsStream << m_LocalNodeId;

		rv = m_LocalDeviceId.Encode(mbBlock);
		if (QT_OK != rv){
			return rv;
		}

		if (m_byKeyNum > 0 && m_byStatus == 0){
			ListKeyID::iterator it = m_KeyIdList.begin();
			ListKey::iterator itKey = m_KeyList.begin();
			for (; it != m_KeyIdList.end() && itKey != m_KeyList.end(); it++, itKey++)
			{
				rv = it->Encode(mbBlock);
				if (QT_OK != rv)
				{
					PRINT("KeyIdList.Encode error\n");
					return rv;
				}
				rv = itKey->Encode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
			}
		}

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock /* out */)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);

		bsStream >> m_wRequestID;
		bsStream >> m_byStatus;
		bsStream >> m_byKeyType;
		bsStream >> m_byKeyNum;
		bsStream >> m_byKeyLength;
		bsStream >> m_byReserved;
		bsStream >> m_LocalNodeId;

		rv = m_LocalDeviceId.Decode(mbBlock);
		if (QT_OK != rv){
			return rv;
		}

		if (m_byKeyNum > 0 && m_byStatus == 0){
			CQtKeyId tmpId(DEFAULT_KEY_ID_LEN);
			CQtKey tmpKey(DEFAULT_KEY_LEN);
			for (int i = 0; i < m_byKeyNum; i++)
			{
				tmpId.Clear();
				tmpKey.Clear();
				rv = tmpId.Decode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
				rv = tmpKey.Decode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
				m_KeyIdList.push_back(tmpId);
				m_KeyList.push_back(tmpKey);
			}
		}
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byStatus) +
			sizeof(m_byKeyType) +
			sizeof(m_byKeyNum) +
			sizeof(m_byKeyLength) +
			sizeof(m_byReserved) +
			sizeof(m_LocalNodeId);
		
		dwLen += DEFAULT_DEVICEID_LEN;

		if(m_byStatus == 0)
		{
			dwLen += m_byKeyNum*DEFAULT_KEY_ID_LEN;
			dwLen += m_byKeyNum*DEFAULT_KEY_LEN;
		}

		return dwLen;
	}


	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	WORD GetStatus()
	{
		return m_byStatus;
	}

	BYTE GetKeyType()
	{
		return m_byKeyType;
	}

	BYTE GetKeyNum()
	{
		return m_byKeyNum;
	}

	BYTE GetKeyLength(){
		return m_byKeyLength;
	}
	
	QT_LONG GetLocalNodeID(){
		return m_LocalNodeId;
	}

	CQtDeviceId GetLocalDeviceId(){
		return m_LocalDeviceId;
	}

	ListKeyID GetListKeyID(){
		return m_KeyIdList;
	}
	
	ListKey GetListKey(){
		return m_KeyList;
	}
	
protected:
	WORD m_wRequestID;
	BYTE m_byStatus;
	BYTE m_byKeyType;
	BYTE m_byKeyNum;
	BYTE m_byKeyLength;
	BYTE m_byReserved;
	QT_LONG 	m_LocalNodeId;
	CQtDeviceId m_LocalDeviceId;
	ListKeyID 	m_KeyIdList;
	ListKey 	m_KeyList;
};

#if 0
class CQtKeySynConf : public CQtSessionPDUBase
{
public:
	CQtKeySynConf()
		: m_wRequestID(0)
		, m_byStatus(0)
	{
		m_cType = QT_KEY_SYN_PDU_CONF;
	}

	CQtKeySynConf(WORD wRequestId, BYTE byStatus)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
	{
		m_cType = QT_KEY_SYN_PDU_CONF;	
	}

	virtual ~CQtKeySynConf() {}
	
public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byStatus;

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock /* out */)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);

		bsStream >> m_wRequestID;
		bsStream >> m_byStatus;
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byStatus);
		
		return dwLen;
	}


	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	WORD GetStatus()
	{
		return m_byStatus;
	}

protected:
	WORD m_wRequestID;
	BYTE m_byStatus;
};

class CQtKeyPushReq : public CQtSessionPDUBase
{
public:
	CQtKeyPushReq()
		: m_wRequestID(0)
		, m_byReserved(0)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_REQ;
	}

	CQtKeyPushReq(WORD wRequestId,
			BYTE byKeyType,
			BYTE byKeyNum,
			BYTE byKeyLength,
			BYTE bySpecified,
			QT_LONG &LocalNodeId,
			QT_LONG &RemoteNodeId,
			ListKeyID &KeyIdList)
		: m_wRequestID(wRequestId)
		, m_byReserved(0)
		, m_byKeyType(byKeyType)
		, m_byKeyNum(byKeyNum)
		, m_byKeyLength(byKeyLength)
		, m_LocalNodeId(LocalNodeId)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_REQ;
	}

	virtual ~CQtKeyPushReq() {}

public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byKeyType;
		bsStream << m_byKeyNum;
		bsStream << m_byKeyLength;
		bsStream << m_byReserved;
		bsStream << m_LocalNodeId;
		bsStream << m_RemoteNodeId;
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock /* out */)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if(QT_OK != rv)
		{
		    PRINT("CQtSessionPDUBase:Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream >> m_wRequestID;
		bsStream >> m_byKeyType;
		bsStream >> m_byKeyNum;
		bsStream >> m_byKeyLength;
		bsStream >> m_byReserved;
		bsStream >> m_LocalNodeId;
		bsStream >> m_RemoteNodeId;

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byReserved) +
			sizeof(m_byKeyType) +
			sizeof(m_byKeyNum) +
			sizeof(m_byKeyLength) +
			sizeof(m_LocalNodeId) +
			sizeof(m_RemoteNodeId);

		return dwLen;
	}
	

	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	BYTE GetKeyType()
	{
		return m_byKeyType;
	}

	BYTE GetKeyNum()
	{
		return m_byKeyNum;
	}

	BYTE GetKeyLength(){
		return m_byKeyLength;
	}
	
	QT_LONG GetLocalNodeID(){
		return m_LocalNodeId;
	}
	
	QT_LONG GetRemotoNodeID(){
		return m_RemoteNodeId;
	}
	
protected:
	WORD m_wRequestID;
	BYTE m_byKeyType;
	BYTE m_byKeyNum;
	BYTE m_byKeyLength;
	WORD m_byReserved;
	QT_LONG m_LocalNodeId;
	QT_LONG m_RemoteNodeId;
};

class CQtKeyAnotherResp : public CQtSessionPDUBase
{
public:
	CQtKeyAnotherResp()
		: m_wRequestID(0)
		, m_byStatus(0)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
		, m_byReserved(0)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_ANOTHER_RESP;
	}

	CQtKeyAnotherResp(WORD wRequestId, BYTE byStatus)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
		, m_byKeyType(0)
		, m_byKeyNum(0)
		, m_byKeyLength(0)
		, m_byReserved(0)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_ANOTHER_RESP;	
	}
	
	CQtKeyAnotherResp(WORD wRequestId,
			BYTE byStatus,
			BYTE byKeyType,
			BYTE byKeyNum,
			BYTE byKeyLength,
			QT_LONG &LocalNodeId,
			ListKeyID &KeyIdList,
			BYTE byVersion,
			BYTE byEncryptAlgrithm,
			WORD wDataLength, 
			CQtKeyId &keyId,
			ListKey &KeyList)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
		, m_byKeyType(byKeyType)
		, m_byKeyNum(byKeyNum)
		, m_byKeyLength(byKeyLength)
		, m_byReserved(0)
		, m_KeyIdList(KeyIdList)
		, m_byVersion(byVersion)
		, m_byEncryptAlgrithm(byEncryptAlgrithm)
		, m_wDataLength(wDataLength)
		, m_KeyId(keyId)
		, m_KeyList(KeyList)
		, m_LocalNodeId(LocalNodeId)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_ANOTHER_RESP;
	}
	
	virtual ~CQtKeyAnotherResp() {}
	
public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byStatus;
		bsStream << m_byKeyType;
		bsStream << m_byKeyNum;
		bsStream << m_byKeyLength;
		bsStream << m_byReserved;
		bsStream << m_LocalNodeId;

		if (m_byKeyNum > 0 && m_byStatus == 0){
			for (ListKeyID::iterator it = m_KeyIdList.begin(); it != m_KeyIdList.end(); it++)
			{
				rv = it->Encode(mbBlock);
				if (QT_OK != rv)
				{
					PRINT("KeyIdList.Encode error\n");
					return rv;
				}
			}
		}
		if (m_byStatus == 0 && m_byKeyNum > 0){
			ListKeyID::iterator itKeyId = m_KeyIdList.begin();
			for (;itKeyId != m_KeyIdList.end();itKeyId++)
			{
				rv = itKeyId->Encode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
			}

			bsStream << m_byVersion;
			bsStream << m_byEncryptAlgrithm;
			bsStream << m_wDataLength;
			if(m_byEncryptAlgrithm != 0)
				m_KeyId.Encode(mbBlock);
			ListKey::iterator itKey = m_KeyList.begin();
			for (;itKey != m_KeyList.end();itKey++)
			{
				rv = itKey->Encode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
			}
		}

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);

		bsStream >> m_wRequestID;
		bsStream >> m_byStatus;
		bsStream >> m_byKeyType;
		bsStream >> m_byKeyNum;
		bsStream >> m_byKeyLength;
		bsStream >> m_byReserved;
		bsStream >> m_LocalNodeId;

		if (m_byKeyNum > 0 && m_byStatus <= 1){
			CQtKeyId tmpId(DEFAULT_KEY_ID_LEN);
			CQtKey tmpKey(DEFAULT_KEY_LEN);
			for (int i = 0; i < m_byKeyNum; i++)
			{
				rv = tmpId.Decode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
				m_KeyIdList.push_back(tmpId);
			}
			
			bsStream >> m_byVersion;
			bsStream >> m_byEncryptAlgrithm;
			bsStream >> m_wDataLength;
			if(m_byEncryptAlgrithm != 0)
				m_KeyId.Decode(mbBlock);
			for (int i = 0; i < m_byKeyNum; i++)
			{
				rv = tmpKey.Decode(mbBlock);
				if (QT_OK != rv){
					return rv;
				}
				m_KeyList.push_back(tmpKey);
			}
		}
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byStatus) +
			sizeof(m_byKeyType) +
			sizeof(m_byKeyNum) +
			sizeof(m_byKeyLength) +
			sizeof(m_byReserved) +
			sizeof(m_LocalNodeId);

		if(m_byStatus == 0)
		{
			dwLen += m_byKeyNum*DEFAULT_KEY_ID_LEN;
			dwLen += sizeof(m_byVersion) + sizeof(m_byEncryptAlgrithm) + sizeof(m_wDataLength);
			if(m_byEncryptAlgrithm != 0)
				dwLen += DEFAULT_KEY_ID_LEN;
			dwLen += m_byKeyNum*DEFAULT_KEY_LEN;
		}

		return dwLen;
	}


	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	WORD GetStatus()
	{
		return m_byStatus;
	}

	BYTE GetKeyType()
	{
		return m_byKeyType;
	}

	BYTE GetKeyNum()
	{
		return m_byKeyNum;
	}

	BYTE GetKeyLength(){
		return m_byKeyLength;
	}
	
	QT_LONG GetLocalNodeID(){
		return m_LocalNodeId;
	}

	ListKeyID GetListKeyID(){
		return m_KeyIdList;
	}

	BYTE GetVersion()
	{
		return m_byVersion;
	}

	BYTE GetEncryptAlgrithm()
	{
		return m_byEncryptAlgrithm;
	}
	
	WORD GetDataLength()
	{
		return m_wDataLength;
	}

	CQtKeyId GetKeyId()
	{
		return m_KeyId;
	}
	
protected:
	WORD m_wRequestID;
	BYTE m_byStatus;
	BYTE m_byKeyType;
	BYTE m_byKeyNum;
	BYTE m_byKeyLength;
	BYTE m_byReserved;
	QT_LONG m_LocalNodeId;
	ListKeyID m_KeyIdList;

	BYTE m_byVersion;
	BYTE m_byEncryptAlgrithm;
	WORD m_wDataLength;
	CQtKeyId m_KeyId;
	ListKey m_KeyList;
};

class CQtKeyPushConf : public CQtSessionPDUBase
{
public:
	CQtKeyPushConf()
		: m_wRequestID(0)
		, m_byStatus(0)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_RESP;
	}

	CQtKeyPushConf(WORD wRequestId, BYTE byStatus)
		: m_wRequestID(wRequestId)
		, m_byStatus(byStatus)
	{
		m_cType = QT_KEY_SYN_PDU_PUSH_RESP;	
	}

	virtual ~CQtKeyPushConf() {}
	
public:
	QtResult Encode(CQtMessageBlock& mbBlock)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Encode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Encode error\n");
			return rv;
		}
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream << m_wRequestID;
		bsStream << m_byStatus;

		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock /* out */)
	{
		QtResult rv = QT_OK;
		rv = CQtSessionPDUBase::Decode(mbBlock);
		if (QT_OK != rv)
		{
			PRINT("CQtSessionPDUBase::Decode error\n");
			return rv;
		}
		
		CQtByteStreamNetwork bsStream(mbBlock);

		bsStream >> m_wRequestID;
		bsStream >> m_byStatus;
		
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	DWORD GetLength()
	{
		DWORD dwLen = CQtSessionPDUBase::GetLength() +
			sizeof(m_wRequestID) +
			sizeof(m_byStatus);
		
		return dwLen;
	}


	WORD GetRequestID()
	{
		return m_wRequestID;
	}

	WORD GetStatus()
	{
		return m_byStatus;
	}

protected:
	WORD m_wRequestID;
	BYTE m_byStatus;
};
#endif

#endif


