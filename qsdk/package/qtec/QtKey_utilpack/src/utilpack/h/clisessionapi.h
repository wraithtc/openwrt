#ifndef __CLI_SESSION_API_H
#define __CLI_SESSION_API_H

#include <string>
#include "networkbase.h"

#ifndef QT_OS_EXPORT
#if defined (_USRDLL)
#define QT_OS_EXPORT __declspec(dllexport)
#else 
#define QT_OS_EXPORT __declspec(dllimport)
#endif // _USRDLL || QT_OS_BUILD_DLL
#endif
/***************************************************************
 *	
 * Description: Net Session Sink
 *
 **************************************************************/
class QT_OS_EXPORT INetSessionSink
{
public:
	/***************************************************************
	 *	
	 * Description: Indicate upper connecting operateion compelete
	 *
	 * iReason: The result of connecting operation. 
	 *		    0 success, other failed
	 *
	 **************************************************************/
	virtual void OnConnect(int iReason, BYTE byContType) = 0;

	/***************************************************************
	 *	
	 * Description: Indicate upper the data received
	 *
	 * byContType: The type of data. 
	 *			CONT_TYPE_IM_DATA for IM Data
	 *			CONT_TYPE_CHAT for chat data
	 *			CONT_TYPE_AUDIO for audio stream data
	 *			CONT_TYPE_CONTR for audio control info.
	 *
	 * pBuf: The buffer hold the data
	 *
	 * dwLength: The length of data in buffer
	 *
	 **************************************************************/
	virtual void OnReceive(BYTE byContType,	BYTE* pBuf, DWORD dwLength) = 0;
	
	/***************************************************************
	 *	
	 * Description: Indicate upper the connection is disconnected
	 *
	 * iReason: The reason why it is disconnected 
	 *		    defined in "networkbase.h"
	 *
	 **************************************************************/
	virtual void OnDisconnect(int iReason, BYTE byContType) = 0;

	/***************************************************************
	 *	
	 * Description: If the return value SendData call is 
	 *				QT_ERROR_PARTIAL_DATA then the upper can 
	 *              stop sending data until OnSend call. It indicates 
	 *				you can send data again.
	 *
	 **************************************************************/
	virtual void OnSend(BYTE byContType) = 0;

	virtual ~INetSessionSink() { }
};

/***************************************************************
 *	
 * Description: Net Session Base Interface
 *
 **************************************************************/
class QT_OS_EXPORT INetSessionBase 
{
public:
	/***************************************************************
	 *	
	 * Description: Disconnect the connection.
	 *
	 * iReason: The reason why it is disconnected 
	 *		    defined in "networkbase.h"
	 *
	 **************************************************************/
	virtual void Disconnect(int nReason = 0) = 0;

	/***************************************************************
	 *	
	 * Description: Send data
	 *
	 * byContType: The type of data. 
	 *			CONT_TYPE_IM_DATA for IM Data
	 *			CONT_TYPE_CHAT for chat data
	 *			CONT_TYPE_AUDIO for audio stream data
	 *			CONT_TYPE_CONTR for audio control info.
	 *
	 * pBuf: The buffer hold the data
	 *
	 * dwLength: The length of data in buffer
	 *
	 * return value: 0, success
	 *               QTEC_NETWORK_ERROR_BUFFER_FULL, buffer full, 
	 *               will call OnSend later
	 *               other, some error occurs
	 *
	 **************************************************************/
	virtual int SendData(
		BYTE	byContType, 
		BYTE	*pBuf, 
		DWORD	dwLength
		) = 0;

	/***************************************************************
	 *	
	 * Description: Set the sink of session. 
	 *
	 * pSink: The new sink for session.
	 *
	 **************************************************************/
	virtual INetSessionSink* SetSink(INetSessionSink *pSink) = 0;


	/***************************************************************
	 *	
	 * Description: Set option. Such as send buffer length 
	 *
	 * dwOptType: The type of option. defined in "networkbase.h"
	 *
	 * pParam: The parameter of option. defined in "networkbase.h"
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int SetOpt(DWORD dwOptType, void *pParam) = 0;

	/***************************************************************
	 *	
	 * Description: Set option. Such as send buffer length 
	 *
	 * dwOptType: The type of option. defined in "networkbase.h"
	 *
	 * pParam: The parameter of option. defined in "networkbase.h"
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int GetOpt(DWORD dwOptType, void *pParam) = 0;

	/***************************************************************
	 *	
	 * Description: Get the session key. Will generate a new key on 
	 *              first call. Will return same key as first call later
	 *
	 * pSessionKey: OUTPUT, hold the session key pointer. The memory is 
	 *              maintained by session
	 *
	 * pKeyLen: OUTPUT, the length of key. 
	 *
	 **************************************************************/
	virtual void GetSessionKey(BYTE **pSessionKey, int *pKeyLen) = 0;

	/***************************************************************
	 *	
	 * Description: The peer is trying connecting in now. 
	 *              Help the session to determine when peer cannot 
	 *              connect in. Called in inviter in p2p mode
	 *              It will start a timer (about 2 minutes)
	 *				when create a session if it is a inviter's session.
	 *              If the timer elapse, it consider as client cannot 
	 *              connect in and connect to server.
	 *              It will start another timer (about 15 seconds) to 
	 *              determine if peer can connect in after call this.
	 *
	 **************************************************************/
	virtual void PeerTryConnectIn() = 0;

	virtual BOOL ByServer() const = 0;

	virtual void SetBufferSize(DWORD dwNewSize) = 0;
	
	virtual ~INetSessionBase() { }
};

/***************************************************************
 *	
 * Description: Net Session Interface
 *
 **************************************************************/
class QT_OS_EXPORT INetSession : public INetSessionBase
{
public:
	
	/***************************************************************
	 *	
	 * Description: Connect to peer or server
	 *
	 * pServerAddr: The server address in string format 
	 *		   
	 * wServerTcpPort: The tcp port of server
	 *
	 * wServerUdpPort: The udp port of server, can skip if no 
	 *				   udp connection
	 *
	 * pPeerAddr: Peer address in string format, can skip if no 
	 *				   peer-to-peer connection
	 *
	 * wPeerTcpPort: Peer tcp port, can skip if no 
	 *				   peer-to-peer connection
	 *
	 * wPeerUdpPort: Peer udp port, can skip if no 
	 *				   peer-to-peer connection
	 *
	 * return value: 0, will call onconnect later.
	 *               other, some error occurs
	 *
	 **************************************************************/
	virtual int Connect(
		char	*pServerAddr,
		WORD	wServerTcpPort,
		WORD	wServerUdpPort = 0,
		char	*pPeerAddr = NULL,
		WORD	wPeerTcpPort = 0,
		WORD	wPeerUdpPort = 0
	) = 0;

	/***************************************************************
	 *	
	 * Description: Same as upper except transfer initial pdu for upper
	 *			    The server will receive init pdu on a session 
	 *              create indication.
	 *
	 * pInitPdu: The initial pdu 
	 *		   
	 * dwInitPduLen: The length of initial pdu
	 *
	 * Others: Same as upper connect API
	 *
	 **************************************************************/
	virtual int Connect(
		BYTE	*pInitPdu,
		DWORD	dwInitPduLen,
		char	*pServerAddr,
		WORD	wServerTcpPort,
		WORD	wServerUdpPort = 0,
		char	*pPeerAddr = NULL,
		WORD	wPeerTcpPort = 0,
		WORD	wPeerUdpPort = 0
	) = 0;

	virtual ~INetSession() { }
};

/***************************************************************
 *	
 * Description: Net Session Interface
 *
 **************************************************************/
class QT_OS_EXPORT IAVSession : public INetSessionBase
{
public:

	virtual int Connect(
		char	*pControlSvrAddr,
		WORD	wControlSvrTcpPort,
		char	*pAudioSvrAddr,
		WORD	wAudioSvrTcpPort,
		WORD	wAudioSvrUdpPort,
		char	*pVideoSvrAddr,
		WORD	wVideoSvrTcpPort,
		WORD	wVideoSvrUdpPort,
		char	*pControlPeerAddr = NULL,
		WORD	wControlPeerTcpPort = 0,
		char	*pAudioPeerAddr = NULL,
		WORD	wAudioPeerTcpPort = 0,
		WORD	wAudioPeerUdpPort = 0,
		char	*pVideoPeerAddr = NULL,
		WORD	wVideoPeerTcpPort = 0,
		WORD	wVideoPeerUdpPort = 0) = 0;

	virtual int Connect(
		BYTE	*pInitPdu,
		DWORD	dwInitPduLen,
		char	*pControlSvrAddr,
		WORD	wControlSvrTcpPort,
		char	*pAudioSvrAddr,
		WORD	wAudioSvrTcpPort,
		WORD	wAudioSvrUdpPort,
		char	*pVideoSvrAddr,
		WORD	wVideoSvrTcpPort,
		WORD	wVideoSvrUdpPort,
		char	*pControlPeerAddr = NULL,
		WORD	wControlPeerTcpPort = 0,
		char	*pAudioPeerAddr = NULL,
		WORD	wAudioPeerTcpPort = 0,
		WORD	wAudioPeerUdpPort = 0,
		char	*pVideoPeerAddr = NULL,
		WORD	wVideoPeerTcpPort = 0,
		WORD	wVideoPeerUdpPort = 0) = 0;

	/***************************************************************
	 *	
	 * Description: Get Conference Type
	 *              CONF_TYPE_PEER_TO_PEER  
	 *              or
	 *              CONF_TYPE_TELECONFERENCE
	 *
	 **************************************************************/
	virtual BYTE GetConfType() = 0;

	virtual ~IAVSession() { }
};

class QT_OS_EXPORT IVMSession : public INetSessionBase
{
public:
	enum{
		RECORD_MAIL,
		PLAY_MAIL
	};
	/***************************************************************
	 *	
	 * Description: Connect to mail server. 
	 *
	 * SendMailType: The type of record mail or play mail, such as RECORD_MAIL,PLAY_MAIL.
	 *				 If type is RECORD_MAIL, create two TCP connection.
	 *				 If type is PLAY_MAIL, create one TCP connection and one UDP connection.
	 *				 For detail, please look through "Video mail system design"
	 *
	 * pMailSvrRTSPAddr: The mail server address(handle RTSP command) in string format 
	 *		   
	 * wMailSvrRTSPTcpPort: The tcp port of server
	 *
	 * pMailSvrAudioAddr: he mail server address(handle RTP audio data) in string format
	 *
	 * wMailSvrAudioTcpPort: The tcp port of server
	 *
	 * wMailSvrAudioUdpPort: The udp port of server
	 *
	 * pMailSvrVideoAddr: he mail server address(handle RTP video data) in string format
	 *
	 * wMailSvrVideoTcpPort: The tcp port of server
	 *
	 * wMailSvrVideoUdpPort: The udp port of server
	 *
	 * return value: 0, will call onconnect later.
	 *               other, some error occurs
	 *
	**************************************************************/
	virtual int ConnectControl(
		BYTE	SendMailType,
		char*	pMailSvrRTSPAddr,
		WORD	wMailSvrRTSPTcpPort
		) = 0;
	virtual int ConnectData(
		char*	pMailSvrAudioAddr,
		WORD	wMailSvrAudioTcpPort,
		WORD	wMailSvrAudioUdpPort,
		char*	pMailSvrVideoAddr,
		WORD	wMailSvrVideoTcpPort,
		WORD	wMailSvrVideoUdpPort) = 0;

	virtual BYTE GetSendMailType() = 0;
	
	virtual ~IVMSession(){}
};

/***************************************************************
 *	
 * Description: Client session manager
 *
 **************************************************************/
class QT_OS_EXPORT IClientSessionManager
{
public:
	/*************************************************************
	*
	* Description: set PKCS7 CA,
	* cacert:			The PEM encoded CA certificate.
	* clientCert:		The PEM encoded client certificate.
	* clientPrivkey:	The PEM encoded client private key.
	* pw:				The password that is used to encrypt the client's
	* bRSA:				whether user rsa wrapp sessionkey
	* private key. In audioplus, this is the client login pw.
	* return value: 0 success.
	*				other failed.
	**************************************************************/
	virtual int SetPKCS7CA(
		const char* cacert,
		const char* clientCert,
		const char* clientPrivkey,
		const char* pw,
		BOOL bRSA) = 0;
	/***************************************************************
	 *	
	 * Description: Client Bind 
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int Bind() = 0;
	
	virtual int Bind(WORD wTcpPort,	char *pAddr = NULL) = 0;
	
	/***************************************************************
	 *	
	 * Description: Get the bind address and port of Chat Session & IM Session 
	 *
	 * wTcpPort: OUTPUT, The TCP port Bound.
	 *
	 * pAddr: OUTPUT, the address bound. Memory is maintained by manager
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int GetBindAddr (
		WORD *pwTcpPort,
		char **pAddr) = 0;
	

	virtual int GetASBindAddr(
		WORD *pwTcpPort,
		char **pAddr) = 0;
		/***************************************************************
	 *	
	 * Description: Get the bind address and port of Control channel of AV Session 
	 *
	 * pwControlTcpPort: OUTPUT, The TCP port Bound.
	 *
	 * pAddr: OUTPUT, the address bound. Memory is maintained by manager
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int GetControlBindAddr (
		WORD *pwControlTcpPort,
		char **pAddr) = 0;
	
	/***************************************************************
	 *	
	 * Description: Get the bind address and port of Audio channel of AV Session 
	 *
	 * pwAudioTcpPort: OUTPUT, The TCP port Bound.
	 *
	 * pwAudioUdpPort: OUTPUT, The UDP port Bound.
	 *
	 * pAddr: OUTPUT, the address bound. Memory is maintained by manager
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int GetAudioBindAddr (
		WORD *pwAudioTcpPort,
		WORD *pwAudioUdpPort,
		char **pAddr) = 0;

	/***************************************************************
	 *	
	 * Description: Get the bind address and port of Video channel of AV Session 
	 *
	 * pwVideoTcpPort: OUTPUT, The TCP port Bound.
	 *
	 * pwVideoUdpPort: OUTPUT, The UDP port Bound.
	 *
	 * pAddr: OUTPUT, the address bound. Memory is maintained by manager
	 *
	 * return value: 0 success.
	 *               other, failed
	 *
	 **************************************************************/
	virtual int GetVideoBindAddr (
		WORD *pwVideoTcpPort,
		WORD *pwVideoUdpPort,
		char **pAddr) = 0;

	/***************************************************************
	 *	
	 * Description: Create audio session. The inviter should create the 
	 *              session for connected in before invite the invitee. 
	 *
	 * byConfType: The type of conference. Such as CONF_TYPE_PEER_TO_PEER
	 *
	 * dwConfId: The conference id.
	 *
	 * dwUserId: The user id.
	 *
	 * pSink: The session sink.
	 *
	 * bInviter: If it is a inviter in peer to peer mode. In conf mode 
	 *           should left it FALSE.
	 *
	 * pAudioSvrCert: The certificate of audio server public key. Used in   
	 *                CONF_TYPE_TELECONFERENCE mode
	 *
	 * pSessKey: The inviter generate key in session while the invitee    
	 *           gots key by IM channel. So we should set the key for 
	 *           invitee when session init.
	 *
	 * dwSessKeyLen: The length of session key.
	 *
	 * return value: A session pointer if success.
	 *               NULL, failed
	 *
	 **************************************************************/
	virtual IAVSession* CreateAVSession(
		BYTE	byConfType,
		DWORD	dwConfId,
		DWORD	dwUserId,
		DWORD	dwSiteId,
		INetSessionSink *pSink = NULL,
		BOOL	bInviter = 0,
		char	*pAudioSvrCert = NULL,
		BYTE	*pSessKey = NULL, 
		DWORD   dwSessKeyLen = 0,
		DWORD	dwPeerUid = 0,
		DWORD	dwPeerSid = 0,
		BOOL	bNeedSessKey = TRUE,
		BOOL    bNeedVideo = TRUE) = 0;

	/***************************************************************
	 *	
	 * Description: Destroy the audio session. 
	 *
	 * pSess: The session to destroy.
	 *
	 **************************************************************/
	virtual void DestroyAVSession(IAVSession *pSess) = 0;

		/***************************************************************
	 *	
	 * Description: Create video mail session. 
	 *
	 * dwUserId: The user id.
	 *
	 * dwSiteId: The user's side id
	 *
	 * pSink: The session sink.
	 *
	 * pMailSvrCert: The certificate of mail server public key. 
	 *
	 * pSessKey: The inviter generate key in session while the invitee    
	 *           gots key by IM channel. So we should set the key for 
	 *           invitee when session init.
	 *
	 * dwSessKeyLen: The length of session key.
	 *
	 *
	 * return value: A session pointer if success.
	 *               NULL, failed
	 *
	 **************************************************************/
	virtual IVMSession* CreateVMSession(
				DWORD dwUserId,
				DWORD dwSiteId,
				INetSessionSink*	pSink = NULL,
				char	*pMailSvrCert = NULL,
				BYTE	*pSessKey = NULL,
				DWORD dwSessKeyLen = 0,
				BOOL	bNeedSessKey = TRUE,
				BOOL    bNeedVideo = TRUE
				) = 0;

	/***************************************************************
	 *	
	 * Description: Destroy the video mail session. 
	 *
	 * pSess: The session to destroy.
	 *
	 **************************************************************/
	virtual void DestoryVMSession(IVMSession* pSess) = 0;
	/***************************************************************
	 *	
	 * Description: Create IM presence session 
	 *
	 * pSink: The session sink.
	 *
	 * return value: A session pointer if success.
	 *               NULL, failed
	 *
	 **************************************************************/
	virtual INetSession * CreateIMSession(INetSessionSink *pSink = NULL) = 0;
	
	/***************************************************************
	 *	
	 * Description: Destroy the IM session. 
	 *
	 * pSess: The session to destroy.
	 *
	 **************************************************************/
	virtual void DestroyIMSession(INetSession *pSess) = 0;

	/***************************************************************
	 *	
	 * Description: Create chat session. The inviter should create the 
	 *              session for connected in before invite the invitee. 
	 *
	 * dwConfId: The conference id.
	 *
	 * dwUserId: The user id.
	 *
	 * pSink: The session sink.
	 *
	 * bInviter: If it is a inviter.
	 *
	 * pSessKey: The inviter generate key in session while the invitee    
	 *           gots key by IM channel. So we should set the key for 
	 *           invitee when session init.
	 *
	 * dwSessKeyLen: The length of session key.
	 *
	 * return value: A session pointer if success.
	 *               NULL, failed
	 *
	 **************************************************************/
	virtual INetSession * CreateChatSession(
		DWORD	dwConfId,
		DWORD	dwUserId,
		DWORD	dwSiteId,
		INetSessionSink *pSink = NULL,
		BOOL	bInviter = 0,
		BYTE	*pSessKey = NULL, 
		DWORD   dwSessKeyLen = 0,
		DWORD	dwPeerUid = 0,
		DWORD	dwPeerSid = 0) = 0;

	/***************************************************************
	 *	
	 * Description: Destroy the chat session. 
	 *
	 * pSess: The session to destroy.
	 *
	 **************************************************************/
	virtual void DestroyChatSession(INetSession *pSess) = 0;

		/***************************************************************
	 *	
	 * Description: Create data session. The inviter should create the 
	 *              session for connected in before invite the invitee. 
	 *
	 * dwConfId: The conference id.
	 *
	 * dwUserId: The user id.
	 *
	 * pSink: The session sink.
	 *
	 * bInviter: If it is a inviter.
	 *
	 * pSessKey: The inviter generate key in session while the invitee    
	 *           gots key by IM channel. So we should set the key for 
	 *           invitee when session init.
	 *
	 * dwSessKeyLen: The length of session key.
	 *
	 * return value: A session pointer if success.
	 *               NULL, failed
	 *
	 **************************************************************/
	virtual INetSession * CreateDataSession(
		DWORD	dwConfId,
		DWORD	dwUserId,
		DWORD	dwSiteId,
		INetSessionSink *pSink = NULL,
		BOOL	bInviter = 0,
		BYTE	*pSessKey = NULL, 
		DWORD   dwSessKeyLen = 0,
		DWORD	dwPeerUid = 0,
		DWORD	dwPeerSid = 0) = 0;

	/***************************************************************
	 *	
	 * Description: Destroy the data session. 
	 *
	 * pSess: The session to destroy.
	 *
	 **************************************************************/
	virtual void DestroyDataSession(INetSession *pSess) = 0;

	/***************************************************************
	 *	
	 * Description: The manager maintain a table to remember how to 
	 *              link to peer. If the user logout or login again,
	 *              we should call this function to clear the old info 
	 *
	 * dwUserID: User Id to clear.
	 *
	 **************************************************************/
	virtual void ClearUserInfo(
		DWORD dwUserID,
		DWORD dwSiteID) = 0;
	

	/***************************************************************
	 *	
	 * Description: Destroy the manager 
	 *
	 **************************************************************/
	virtual void DestroyManger() = 0;

	virtual ~IClientSessionManager() { }
};

extern "C"
{
	/***************************************************************
	 *	
	 * Description: Create a manager 
	 *
	 **************************************************************/
	QT_OS_EXPORT IClientSessionManager* CreateClientSessionManager();

	/***************************************************************
	 *	
	 * Description: Get authentication info
	 *	
	 * aShow: the string showed in the dialog box	
	 *	
	 * Return value: 0 success, other failed	
	 *
	 **************************************************************/
	QT_OS_EXPORT int GetUserNameAndPasswdBlocked(std::string  &aUserName, std::string &aPassword);

	/***************************************************************
	 *	
	 * Description: Interrupt the blocked operation GetUserNameAndPasswdBlocked.
	 *				Only one dialog can appear in a program.
	 *	
	 **************************************************************/
	QT_OS_EXPORT void InterruptGetUserNameAndPasswd();

};

#endif // __CLI_SESSION_API_H

