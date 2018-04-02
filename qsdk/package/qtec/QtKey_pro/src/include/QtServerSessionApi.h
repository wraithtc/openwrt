    /*------------------------------------------------------*/
/* Server session APIs                                  */
/*                                                      */
/* MmServerSessionApi.h                                 */
/*                                                      */
/* Copyright (C) QTEC  Inc.                             */
/* All rights reserved                                 	*/
/*                                                      */
/* Author                                               */
/*      zhubin(zhubin@qtec.cn)                          */
/*                                                      */
/* History                                              */
/*                                                      */
/*	2017/02/23	Create									*/
/*------------------------------------------------------*/

#ifndef QTSERVERSESSIONAPI_H
#define QTSERVERSESSIONAPI_H

#include "QtServerSessionApiDefines.h"
#include "QtVersion.h"
#include "QtConnectionInterface.h"
#include <sys/time.h>
#include <cstdlib>

class CQtInetAddr;
class IQtServerSession;
class IQtServerSessionAuthSink;
class IQtSessionDataProcessor;
class IQtServerSessionSink;
enum{
	QT_SESSION_NEED_BUFFER = 1
};

typedef BYTE QT_SERVER_SESSION_TYPE;
enum{
	QT_SERVER_SESSION_TYPE_NONE = 0,
	QT_SERVER_SESSION_TYPE_PARENT = 1,
	QT_SERVER_SESSION_TYPE_CHILD = 2,
	QT_SERVER_SESSION_TYPE_CLIENT = 3,
	QT_SERVER_SESSION_TYPE_CENTER = 4,
	QT_SERVER_SESSION_TYPE_NODECTRL = 5
};

template <unsigned char bufferLen = 32>
class QT_OS_EXPORT CQtKeyT
{
public:
	CQtKeyT() : m_byDataLen(bufferLen)
	{
		memset(m_buffer, 0, sizeof(m_buffer));
	}

	CQtKeyT(int dataLen) : m_byDataLen(dataLen)
	{
		if (m_byDataLen > bufferLen){
			m_byDataLen = bufferLen;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
	}

	CQtKeyT(BYTE* pId, BYTE byIdLen)
	{
		if (!pId || byIdLen > bufferLen || byIdLen == 0){
			m_byDataLen = bufferLen;
			memset(m_buffer, 0, sizeof(m_buffer));
		}
		else{
			memset(m_buffer, 0, sizeof(m_buffer));
			memcpy(m_buffer, pId, byIdLen);
			m_byDataLen = byIdLen;
		}
	}

	void Clear() { memset(m_buffer, 0, sizeof(m_buffer)); }
	
	void Xor(const CQtKeyT &other){
		unsigned int *pMyBuff = (unsigned int *)m_buffer;
		unsigned int *pOther = (unsigned int *)other.m_buffer;
		for(int i=0; i<bufferLen; i+=4){
			*pMyBuff = (*pMyBuff) xor (*pOther);
			pMyBuff++;
			pOther++;
		}
	}

	QtResult SetValue(BYTE* pValue, BYTE byLen) { 
		if (!pValue || byLen > bufferLen || byLen == 0){
			return QT_ERROR_FAILURE;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
		memcpy(m_buffer, pValue, byLen);
		m_byDataLen = byLen;
		return QT_OK;
	}

	QtResult RandomStrValue(BYTE byLen) { 
		if (byLen > bufferLen || byLen == 0){
			return QT_ERROR_FAILURE;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
		m_byDataLen = byLen;
		char *pChars = "0123456789abcdefghijkmnlopqrstuvwxyzABCDEFGHIJKMNLOPQRSTUVWXYZ-_";
		struct timeval tv; 
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec + tv.tv_sec<<16);
		for (int i = 0; i < m_byDataLen; i++)
		{
			int j = rand() & 0x3F;
			m_buffer[i] =(BYTE)pChars[j];
		}
		
		return QT_OK;
	}

	QtResult RandomValue(BYTE byLen) { 
		if (byLen > bufferLen || byLen == 0){
			return QT_ERROR_FAILURE;
		}
		memset(m_buffer, 0, sizeof(m_buffer));
		m_byDataLen = byLen;
		BYTE rest = byLen & 0x03;
		struct timeval tv; 
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec + tv.tv_sec<<16);
		if(rest > 0){
			for(int i=0; i<rest; i++){
				m_buffer[i] = (BYTE)(rand() & 0xFF);
			}
		}
		int *pIntBuff = (int *)(m_buffer+rest);
		for (int i = 0; i < byLen; i+=4)
		{
			*pIntBuff = rand();
			pIntBuff++;
		}
		
		return QT_OK;
	}
	
	BYTE* GetBuffer() { return m_buffer; }
	BYTE GetDataLen(){ return m_byDataLen; }


	QtResult Encode(CQtMessageBlock& mbBlock) const
	{
		CQtByteStreamNetwork bsStream(mbBlock);
		bsStream.Write(m_buffer, m_byDataLen);
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	QtResult Decode(CQtMessageBlock& mbBlock)
	{
		CQtByteStreamNetwork bsStream(mbBlock);

		bsStream.Read(m_buffer, m_byDataLen);
		if (!bsStream.IsGood())
		{
			return QT_ERROR_FAILURE;
		}

		return QT_OK;
	}

	bool operator == (const CQtKeyT &aRight) const
	{
		if (m_byDataLen != aRight.m_byDataLen){
			return false;
		}
		if (memcmp(m_buffer, aRight.m_buffer, m_byDataLen) == 0){
			return true;
		}

		return false;
	}

	bool operator != (const CQtKeyT &aRight) const
	{
		if (m_byDataLen != aRight.m_byDataLen){
			return true;
		}
		if (memcmp(m_buffer, aRight.m_buffer, m_byDataLen) == 0){
			return false;
		}

		return true;
	}

	bool operator < (const CQtKeyT &aRight) const
	{
		if (m_byDataLen < aRight.m_byDataLen){
			return true;
		}

		if (m_byDataLen > aRight.m_byDataLen){
			return false;
		}

		if (memcmp(m_buffer, aRight.m_buffer, m_byDataLen) < 0){
			return true;
		}

		return false;
	}

	bool operator >(const CQtKeyT &aRight) const
	{
		if (m_byDataLen < aRight.m_byDataLen){
			return false;
		}

		if (m_byDataLen > aRight.m_byDataLen){
			return true;
		}

		if (memcmp(m_buffer, aRight.m_buffer, m_byDataLen) > 0){
			return true;
		}

		return false;
	}

	CQtKeyT&  operator=(const CQtKeyT &aRight)
	{	
	
		m_byDataLen = aRight.m_byDataLen;

		memcpy(m_buffer, aRight.m_buffer, bufferLen) ;

		return *this;
	}
	
private:
	BYTE m_buffer[bufferLen];
	BYTE m_byDataLen;
};

typedef CQtKeyT<DEFAULT_KEY_ID_LEN> CQtKeyId;
typedef CQtKeyT<DEFAULT_KEY_LEN> CQtKey;
typedef CQtKeyT<DEFAULT_DEVICEID_LEN> CQtDeviceId;
typedef CQtKeyT<DEFAULT_NODE_ID_LEN> CQtNodeId;
typedef CQtKeyT<DEFAULT_USER_NAME_LEN> CQtUserName;
typedef CQtKeyT<DEFAULT_USER_ID_LEN> CQtUserId;
typedef CQtKeyT<DEFAULT_SESSION_ID_LEN> CQtSessionId;
typedef CQtKeyT<DEFAULT_SESSION_NAME_LEN> CQtSessionName;

/*
class CQtSessionInitMessageEvent : public IQtEvent
{
public:
	CQtSessionInitMessageEvent(IQtSessionDataProcessor *pDataProcessor,
		CQtMessageBlock &aData,
		IQtTransport *pTransport)
		: m_pDataProcessor(pDataProcessor)
		, m_pSessionData(aData.DuplicateChained())
		, m_pTransport(pTransport)
	{
	}

	virtual ~CQtSessionInitMessageEvent()
	{
		if (m_pSessionData) {
			m_pSessionData->DestroyChained();
			m_pSessionData = NULL;
		}
	}

	// interface IQtEvent
	virtual QtResult OnEventFire()
	{
		if (m_pDataProcessor)
			return m_pDataProcessor->ProcessMessage(*m_pSessionData, m_pTransport);
		return QT_ERROR_FAILURE;
	}

protected:
	IQtSessionDataProcessor *m_pDataProcessor;
	CQtMessageBlock *m_pSessionData;
	IQtTransport *m_pTransport;
};
*/

class QT_OS_EXPORT IQtHeartbeatSink
{
	public:   
		IQtHeartbeatSink() { }
		~IQtHeartbeatSink() { }
		virtual void OnHeartbeatError(IQtServerSession *pSession, QT_SERVER_SESSION_TYPE aType, LPVOID pUserData) = 0;	
};



	class QT_OS_EXPORT IQtServerSessionSink
	{
	public:
		virtual void OnReceive(
			CQtMessageBlock &aData,
			IQtServerSession *aSession) = 0;
			
		virtual void OnSessionCreated(IQtServerSession* pSession,
			QT_SERVER_SESSION_TYPE aType) = 0;
			
		virtual void OnSessionDisconnect(IQtServerSession* pSession, QT_SERVER_SESSION_TYPE aType = QT_SERVER_SESSION_TYPE_NONE) = 0;
	
		virtual ~IQtServerSessionSink() { }
	};

class CQtSessionMessageEvent : public IQtEvent
{
public:
	CQtSessionMessageEvent(IQtServerSessionSink *pSessionSink,
		CQtMessageBlock &aData,
		IQtServerSession *pSession)
		: m_pSessionSink(pSessionSink)
		, m_pSessionData(aData.DuplicateChained())
		, m_pSession(pSession)
	{
	}

	virtual ~CQtSessionMessageEvent()
	{
		if (m_pSessionData) {
			m_pSessionData->DestroyChained();
			m_pSessionData = NULL;
		}
	}

	// interface IQtEvent
	virtual QtResult OnEventFire()
	{
		if (m_pSessionSink)
		{
			m_pSessionSink->OnReceive(*m_pSessionData, m_pSession);
			return QT_OK;
		}

		return QT_ERROR_FAILURE;
	}

protected:
	IQtServerSessionSink *m_pSessionSink;
	CQtMessageBlock *m_pSessionData;
	IQtServerSession *m_pSession;
};

// Note: all the functions must be called in the same thread.
class QT_OS_EXPORT CQtServerSessionAPI
{
public:
	// listen on the control and data addresses.
	static QtResult ListenContorlAndDataAddr(
		const CQtInetAddr &aTcpAddr,
		const CQtInetAddr &aUdpAddr,
		IQtServerSessionAuthSink *aAuthSink);
	
	// Create session in this server.
	// return session-id as <aSessionId>.
	static QtResult CreateSession(
		CQtString strUserName,
		CQtKeyId sessKeyId,
		CQtKey sessKey,
		IQtTransport *pTransport,
		IQtServerSessionSink *aSink, 
		IQtServerSession *&aSession);

	static QtResult DeleteSession(
		IQtServerSession *&aSession);
};


class QT_OS_EXPORT IQtServerSession
	: public IQtTransportSink,
	 public IQtEvent
{
public:
	IQtServerSession()
	{
	}
	virtual ~IQtServerSession() { }
	// It must be called by Host.
	virtual QtResult StartSession() = 0;
	virtual QtResult StopSession() = 0;

	// Send data to data transport.
	// Not implemented in the current version.
	virtual QtResult SendData(
		LPBYTE aData,
		DWORD aLength,
		DWORD indicate = QT_SESSION_NEED_BUFFER) = 0;

	virtual QtResult SendData(
		CQtMessageBlock &aData,
		DWORD indicate = QT_SESSION_NEED_BUFFER
		) = 0;

	// Set option for session.
	virtual QtResult SetOption(DWORD aType, LPVOID aValue) = 0;

	// Get option for session.
	virtual QtResult GetOption(DWORD aType, LPVOID aValue) = 0;

	// interface IQtTransportSink
	virtual void OnReceive(
		CQtMessageBlock &aData,
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL) = 0;

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId) = 0;

	virtual IQtTransport * GetTransport() = 0;

};

class QT_OS_EXPORT IQtServerSessionAcceptorSink
{
public:
	virtual void OnReceiveData(
		LPBYTE aData,
		DWORD aLength) = 0;
	virtual void OnConnectionIndication(IQtTransport *pTransport) = 0;
	virtual ~IQtServerSessionAcceptorSink() { }
};

class QT_OS_EXPORT IQtSessionDataProcessor
{
public:
	virtual QtResult ProcessMessage(
		CQtMessageBlock &aData, IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL) = 0;
	virtual ~IQtSessionDataProcessor() { }
};

class QT_OS_EXPORT CQtRandString
{
public:
	CQtRandString()	
	{
	}
	
	~CQtRandString() { }
	 
	INT Rand_str(BYTE *str,const int len)
	{
	    int i;

		struct timeval tv; 
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec + tv.tv_sec<<16);

	    for(i=0; i<len; i++)
	    {
	        switch(rand() % 3)
	        {
	            case 0:
	                str[i] = 'A' + rand() % 26;
	                break;
	            case 1:
	                str[i] = '0' + rand() % 10;
	                break;
	            case 2:
	                str[i] = 'a' + rand() % 26;
	                break;
	        }
	    }
	    str[i++]= '\0';
	    return 0;
	}
};
 class CQtAuthSessionInfo
 {
 public:
 	CQtAuthSessionInfo()
 	{
 	}
	
	CQtAuthSessionInfo(CQtString strUserName, IQtTransport *pTransport)
	{
		m_strUserName = strUserName;
		m_pTransport = pTransport;
	}

	~CQtAuthSessionInfo()
	{
	}

	CQtString GetUserName()
	{
		return m_strUserName;
	}

	IQtTransport *GetTransport()
	{
		return m_pTransport;
	}

	bool operator == (const CQtAuthSessionInfo &aRight) const
	{
		if(m_strUserName.length() != aRight.m_strUserName.length()){
			return false;
		}
		
		if (memcmp(m_strUserName.data(), aRight.m_strUserName.data(), m_strUserName.length()) == 0
		&& m_pTransport == aRight.m_pTransport){
			return true;
		}
		
		return false;
	}
	 
	 bool operator < (const CQtAuthSessionInfo &aRight) const
	 {
		 if (strcmp(m_strUserName.data(), aRight.m_strUserName.data()) < 0)
		 {
			 return true;
		 }
		 else if(strcmp(m_strUserName.data(), aRight.m_strUserName.data()) == 0)
		 {
			/*if(m_pTransport < aRight.m_pTransport)
				return true;
			else*/
				return false;
		 }
		 else
		 {
		 	return false;
		 }
	 }
	 
	 bool operator >(const CQtAuthSessionInfo &aRight) const
	 {
		 if (strcmp(m_strUserName.data(), aRight.m_strUserName.data()) > 0)
		 {
			 return true;
		 }
		 else if(strcmp(m_strUserName.data(), aRight.m_strUserName.data()) == 0)
		 {
			/*if(m_pTransport > aRight.m_pTransport)
				return true;
			else*/
				return false;
		 }
		 else
		 {
		 	return false;
		 }
	 }

	 CQtAuthSessionInfo&  operator=(const CQtAuthSessionInfo &aRight)
	 {	 
	 
		m_strUserName = aRight.m_strUserName;

		m_pTransport = aRight.m_pTransport;
		
		return *this;
	 }
		
 private:
	 CQtString m_strUserName;
	 IQtTransport *m_pTransport;
 };


 void PrintMessageBlock(CQtMessageBlock &aData);

 void PrintBuffer(const BYTE *pBuff1, int nLen);

  void PrintBufferAsStr(const BYTE *pBuff1, int nLen);


#endif // MMSERVERSESSIONAPI_H
