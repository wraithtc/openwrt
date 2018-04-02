/*------------------------------------------------------*/
/* Error and option code for network module             */
/*                                                      */
/* QtErrorNetwork.h                                     */
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

#ifndef QTERRORNETWORK_H
#define QTERRORNETWORK_H

#include "QtError.h"

enum {
	QT_ERROR_NETWORK_SOCKET_ERROR = QT_ERROR_MODULE_NETWORK + 1, 
	QT_ERROR_NETWORK_SOCKET_RESET,
	QT_ERROR_NETWORK_SOCKET_CLOSE,
	QT_ERROR_NETWORK_SOCKET_BIND_ERROR,
	QT_ERROR_NETWORK_CONNECT_ERROR,
	QT_ERROR_NETWORK_CONNECT_TIMEOUT,
	QT_ERROR_NETWORK_DNS_FAILURE,
	QT_ERROR_NETWORK_PROXY_SERVER_UNAVAILABLE,
	QT_ERROR_NETWORK_UNKNOWN_ERROR,
	QT_ERROR_NETWORK_NO_SERVICE,

	///////////For protocol tp Service/////////////
	QT_ERROR_NETWORK_CONNECTION_RECONNECT, 
	QT_ERROR_NETWORK_CONNECTION_RECONNECT_FAILED, 
	QT_ERROR_NETWORK_CONNECTION_WRONG_TYPE, 
	QT_ERROR_NETWORK_ABATE_CONNECTION,
	QT_ERROR_NETWORK_DENY_ERROR,
	
	QT_ERROR_NETWORK_IMCOMPLETE_CONNECTION,/// for group connection, such as jambosession
	QT_ERROR_NETWORK_RETRY_ERROR,	// overflow retry times 
	QT_ERROR_NETWORK_PDU_ERROR,
	QT_ERROR_PROXY_RETRYTIMES_OVER,
	QT_ERROR_PROXY_CACNEL_BY_USER,
	QT_ERROR_NETWORK_NO_PROXY,
	QT_ERROR_NETWORK_RECEIVED_NONE //server not get any packets over 30 seconds, failover Mar 23 2008 Victor
};


#define QT_OPT_TRANSPORT_BASE			100

//Param. is Pointer to DWORD
//IO Read len
#define QT_OPT_TRANSPORT_FIO_NREAD		(QT_OPT_TRANSPORT_BASE+1)

//Param. is Pointer to DWORD
//Transport unread len
#define QT_OPT_TRANSPORT_TRAN_NREAD		(QT_OPT_TRANSPORT_BASE+2)

//Param. is Pointer to QT_HANDLE
//Get fd
#define QT_OPT_TRANSPORT_FD				(QT_OPT_TRANSPORT_BASE+3)

//Param. is Pointer to CInetAddr
//Get address socket binded
#define QT_OPT_TRANSPORT_LOCAL_ADDR		(QT_OPT_TRANSPORT_BASE+4)

//Param. is Pointer to CInetAddr
//Get peer addr
#define QT_OPT_TRANSPORT_PEER_ADDR		(QT_OPT_TRANSPORT_BASE+5)

//If the socket is still alive
//Param. is Pointer to BOOL
#define QT_OPT_TRANSPORT_SOCK_ALIVE		(QT_OPT_TRANSPORT_BASE+6)

//Param. is Pointer to DWORD
//TYPE_TCP, TYPE_UDP, TYPE_SSL...
#define QT_OPT_TRANSPORT_TRAN_TYPE 		(QT_OPT_TRANSPORT_BASE+7)

// budingc add to support SO_KEEPALIVE function
//Param. is Pointer to DWORD
#define QT_OPT_TRANSPORT_TCP_KEEPALIVE	(QT_OPT_TRANSPORT_BASE+8)

//Param. is Pointer to DWORD
#define QT_OPT_TRANSPORT_RCV_BUF_LEN	(QT_OPT_TRANSPORT_BASE+9)

//Param. is Pointer to DWORD
#define QT_OPT_TRANSPORT_SND_BUF_LEN 	(QT_OPT_TRANSPORT_BASE+10)

//Param. is Pointer to CQtMessageBlock*, 
//send some user data in the connect request
#define QT_OPT_CONNECTION_CONNCET_DATA 	(QT_OPT_TRANSPORT_BASE+11)

//Param. is Pointer to IQtTransport** ,
//get lower transport, for class CQtTransportThreadProxy
#define QT_OPT_LOWER_TRANSPORT	 		(QT_OPT_TRANSPORT_BASE+12)

//Param. is Pointer to int ,
//for DSCP setting, Type of Service (TOS) settings.
#define QT_OPT_TRANSPORT_TOS	 		(QT_OPT_TRANSPORT_BASE+13)

///////////For Connection Service/////////////
//Param. is Pointer to BOOL
#define CS_OPT_NEED_KEEPALIVE			(QT_OPT_TRANSPORT_BASE+31)

//Param. is Pointer to DWORD
#define CS_OPT_MAX_SENDBUF_LEN			(QT_OPT_TRANSPORT_BASE+32)

//Param. is Pointer to BOOL
#define CS_OPT_PKG_NEED_BUF				(QT_OPT_TRANSPORT_BASE+33)

//Param. is Pointer to DWORD (Seconds)
#define CS_OPT_KEEPALIVE_INTERVAL		(QT_OPT_TRANSPORT_BASE+34)
//Param. is Pointer to BOOL 
#define CS_OPT_DISABLE_RCVDATA_FLAG		(QT_OPT_TRANSPORT_BASE+35)

//Param. is for connection abate overflow time, WORD
#define CS_OPT_ABATE_TIME				(QT_OPT_TRANSPORT_BASE+36)

//Param. is for connection reconnect overflow time, WORD
#define CS_OPT_RECONNECT_TIME			(QT_OPT_TRANSPORT_BASE+37)

//Param. inquire the RTT, BOOL
#define CS_OPT_DETECT_RTT				(QT_OPT_TRANSPORT_BASE+38)
//Param. inquire the RTT
#define CS_OPT_INQUIRE_RTT				(QT_OPT_TRANSPORT_BASE+39)

#define CS_OPT_SERVER_UNAVAIL_TIMEOUT	(QT_OPT_TRANSPORT_BASE+40)

//////////////////////////////////////////////

/// for channel
#define QT_OPT_CHANNEL_BASE				200

//Param. is Pointer to BOOL
#define QT_OPT_CHANNEL_FILE_SYNC		(QT_OPT_CHANNEL_BASE + 101)

//Param. is Pointer to BOOL
#define QT_OPT_CHANNEL_HTTP_HEADER_NO_CONTENT_LENGTH	(QT_OPT_CHANNEL_BASE + 111)

//Param. is Pointer to BOOL
#define QT_OPT_CHANNEL_HTTP_PARSER_SKIP_CONTENT_LENGTH	(QT_OPT_CHANNEL_BASE + 112)

#define QT_OPT_CHANNEL_HTTP_APPEND_HTTPHEAD_FOR_EACH	(QT_OPT_CHANNEL_BASE + 113)

//#define QT_OPT_CHANNEL_HTTP_DELEVER_PARTIAL_DATA		(QT_OPT_CHANNEL_BASE + 114)

#define QT_OPT_CHANNEL_HTTP_LOCK_ADDRESS				(QT_OPT_CHANNEL_BASE + 115)

#endif // QTERRORNETWORK_H
