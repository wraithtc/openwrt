#ifndef __CONNECTION_INTERFACE_H
#define __CONNECTION_INTERFACE_H

#include "networkbase.h"
#include "cmcrypto.h"
#include "QtConnectionInterface.h"

class IConnection;
class IConnectionSink;
class IConAcceptor;
class IConAcceptorSink;

class QT_OS_EXPORT IConnectionManager
{
public:

	// pSink should not be NULL.
	virtual IConAcceptor* CreateConAcceptor(
		IConAcceptorSink *pSink, 
		CQtConnectionManager::CType cType
		) = 0;

	virtual IConnection* CreateConnection(
		IConnectionSink *pSink, 
		BOOL bIsDetection, //Add for detection connector
		CQtConnectionManager::CType cType, 
		BYTE byContType = CONT_TYPE_IM_DATA
		) = 0;

	virtual void DestroyConAcceptor(IConAcceptor *pApt) =0;
	virtual void DestroyConnection(IConnection *pCon) = 0;

	//When acceptor indicate a connection to up layer, the up layer should set the sink
	virtual void SetConnectionSink(
		IConnection *pCon, 
		IConnectionSink *pSink 
		) = 0;
	
	virtual int SetPKCS7CA(const char* cacert,
							const char* clicert,
							const char* cliprivatekey,
							const char* clipasswd,
							BOOL bRSA) = 0;

	virtual IQtCrypto* GetCrypto() = 0;

	virtual void DestroyManager() = 0;

	virtual void SetLocalIP(LPCSTR szLocalIP) = 0;
	virtual CQtInetAddr& GetLocalIP() = 0;
};

class QT_OS_EXPORT IConAcceptorSink
{
public:
	//pConParam: For IM connection, it is NULL.
	//			 For Audio Connection, it contains conf_id and user_id
	virtual int OnConnectionCreate (
		IConnection	*pCon, 
		BYTE		byContType, 
		BYTE		*pData,
		DWORD		dwLength) = 0;
};

class QT_OS_EXPORT IConAcceptor
{
public:
	//pAddr: The address to listen. If it is NULL, 
	//		 listen on INADDR_ANY.
	//nPort: The port to listen. 
	//bPortAutoSearch: If 1, will try other port if cannot bind to wPort  
	//return value: The port bind, -1 failed.
	virtual int StartListen(
		const char	*pAddr, 
		WORD		wPort, 
		BOOL		bPortAutoSearch = 0) = 0;

	virtual int StopListen(int iReason = 0) = 0;
};

class QT_OS_EXPORT IConnectionSink
{
public:
	// nReason is like IConnectionManager::REASON_SUCCESSFUL, etc.
	virtual int OnConnect(int iReason) = 0;

	// nReason is like IConnectionManager::REASON_SERVER_UNAVAILABLE, etc.
	// and it will never be IConnectionManager::REASON_SUCCESSFUL
	virtual int OnDisconnect(int iReason) = 0;

	//Recevie data
	virtual int OnReceive(
		BYTE	byContType,
		BYTE	*pData, 
		DWORD	dwLength
		) = 0;

	virtual int OnSend()=0;
};

class QT_OS_EXPORT IConnection
{
public:
	// if succeeds return 0, otherwise return -1.
	//pProxySetting: May be used in future. Proxy setting may be set in UI. 
	//				Then it	should be delivered to lower layer 
	//pData: The data of upper layer.
	//		 Fox an example,it contains conf_id and user_id for audio connection
	virtual int Connect(
		const char	*pSrvAddr, 
		WORD		wSrvPort,
		void		*pProxySetting,
		BYTE		*pData,
		DWORD		dwLength
		) = 0;

	///////////////////Add for detection connector/////////////
	virtual int AddConnection(
		CQtConnectionManager::CType cType, 
		const char	*pAddr, 
		WORD		wPort,
		void		*pProxySetting) = 0;

	virtual int StartDetectionConnect(
		BYTE	*pData,
		DWORD	dwLength,
		BYTE	*pDataEx = NULL,
		DWORD	dwLengthEx = 0) = 0;
	///////////////////////////////////////////////////////////

	// always succeeds
	virtual void Disconnect(int iReason = 0) = 0;

	// if succeeds return 0, otherwise return -1.
	virtual int SendData(
		BYTE	byContType,
		BYTE	*pData, 
		DWORD	dwLength
		) = 0;
	
	//The lost rate of receivers
	virtual float GetLostRate() = 0;

	//The mask of data to be received by sink. For audio server, it can only receives 
	//the data it concerns such as only control data.
	virtual void SetReceiveMask(BYTE byMask)=0;

	virtual int SetOpt(DWORD OptType, void *pParam) = 0;

	virtual int GetOpt(DWORD OptType, void *pParam) = 0;

	virtual ~IConnection() { }
};

extern "C"
{
	QT_OS_EXPORT IConnectionManager *CreateConnectionManager();
};
#endif // !__CONNECTION_INTERFACE_H

