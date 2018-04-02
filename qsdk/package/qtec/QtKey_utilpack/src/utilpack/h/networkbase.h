#ifndef __NETWORK_BASE_H
#define __NETWORK_BASE_H

#define CONF_TYPE_NONE				0x00
#define CONF_TYPE_PEER_TO_PEER		0x01	//Audio+Video
#define CONF_TYPE_AUDIO_CONF		0x02	//Audio
#define CONF_TYPE_TELECONFERENCE	0x04	//Telephony
#define CONF_TYPE_CHAT				0x08
#define CONF_TYPE_AS				0x10
#define CONF_TYPE_VM				0x20

#define CONT_TYPE_IM_DATA			0x01
#define CONT_TYPE_AUDIO				0x02
#define CONT_TYPE_VIDEO				0x04
#define CONT_TYPE_CONTR				0x08
#define CONT_TYPE_CHAT				0x10
#define CONT_TYPE_AS				0x20
#define CONT_TYPE_VM				0x40

#ifndef MACOS
typedef unsigned long	DWORD;
typedef unsigned char	BYTE;
#endif
enum 
{ 
	TYPE_UDP, 
	TYPE_TCP, 
	TYPE_SSL, 
	TYPE_HTTP 
};

enum 
{
	REASON_SUCCESSFUL = 0,
	REASON_SERVER_UNAVAILABLE, 
	REASON_SERVER_DISCONNECT,
	REASON_PEER_DISCONNECT,
	REASON_SERVER_ERROR,
	REASON_SOCKET_ERROR,
	REASON_BIND_ERROR,
	REASON_KEEPALIVE_TIMEOUT,
	REASON_SERVER_DELETE,
	REASON_CONNECT_TIMEOUT,
	REASON_DNS_ERROR,
	REASON_PEER_REJECT,
	REASON_SERVER_USER_DEL_PENDING,
	REASON_SERVER_LOAD_OVERFLOW,
	REASON_UNKNOWN_ERROR,
	REASON_INCORRECT_SESSIONKEY,
};


#define QTEC_NETWORK_ERROR_BASE					40000

#define QTEC_NETWORK_ERROR_CONTROL				(QTEC_NETWORK_ERROR_BASE+1)
#define QTEC_NETWORK_ERROR_AUDIO					(QTEC_NETWORK_ERROR_BASE+2)
#define QTEC_NETWORK_ERROR_VIDEO					(QTEC_NETWORK_ERROR_BASE+3)


/***********************************************************************
* Set Options
 ***********************************************************************/
struct WbxNetOptSetKey
{
	DWORD	dwkeyLen;
	BYTE	*pKey;
};

#define QTEC_NETWORK_OPTION_BASE					200
//Parameter is pointer to WbxNetOptSetKey, 
//will enable encrypt automatically after set the key
#define QTEC_NET_OPT_TYPE_SET_KEY				(QTEC_NETWORK_OPTION_BASE+1)
//Parameter is pointer to long, if 1 enabled, 0 disabled
#define QTEC_NET_OPT_TYPE_ENABLE_ENCRYPT			(QTEC_NETWORK_OPTION_BASE+2)

//Param. is Pointer to DWORD
#define QTEC_TRANSPORT_OPT_SET_RCV_BUF_LEN		(QTEC_NETWORK_OPTION_BASE+3)
//Param. is Pointer to DWORD
#define QTEC_TRANSPORT_OPT_SET_SND_BUF_LEN 		(QTEC_NETWORK_OPTION_BASE+4)

/***********************************************************************
* Get Options
 ***********************************************************************/

//Parameter is a pointer to ulong
enum
{
	CON_TYPE_BASE = 0,
	CON_TYPE_CONNECT_TO_PEER,
	CON_TYPE_PEER_CONNECT_IN,
	CON_TYPE_CONNECT_TO_SERVER
};
#define QTEC_NET_OPT_TYPE_GET_CON_TYPE			(QTEC_NETWORK_OPTION_BASE+5)

//Parameter is a pointer to DWORD
//TYPE_TCP, TYPE_UDP ...
#define QTEC_NET_OPT_TYPE_GET_TRANS_TYPE			(QTEC_NETWORK_OPTION_BASE+6)

//Parameter is a pointer to DWORD
//TYPE_TCP, TYPE_UDP ...
#define QTEC_NET_OPT_TYPE_GET_CONTROL_TRANS_TYPE	(QTEC_NETWORK_OPTION_BASE+7)
#define QTEC_NET_OPT_TYPE_GET_AUDIO_TRANS_TYPE	(QTEC_NETWORK_OPTION_BASE+8)
#define QTEC_NET_OPT_TYPE_GET_VIDEO_TRANS_TYPE	(QTEC_NETWORK_OPTION_BASE+9)

#endif// __NETWORK_BASE_H

