/*------------------------------------------------------*/
/* Session APIs defines (for client and server)         */
/*                                                      */
/* MmSessionApiDefines.h                                */
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


#ifndef QT_SESSIONAPIDEFINES_H
#define QT_SESSIONAPIDEFINES_H

#define QT_ENABLE_RELIABLE_CONNECTION

#include "QtDefines.h"
#include "QtStdCpp.h"

class CCmMessageBlock;

static const WORD gNoGetAnyPkg = 10000;	//for no received any packags in sum cycle

const DWORD QTS_ROLE_HOST				= 0x0001 << 31;
const DWORD QTS_ROLE_PRESENTER			= 0x0001 << 30;

#define IS_PRESENTER(role)  (role & QTS_ROLE_PRESENTER)
#define IS_HOST( role) ( role & QTS_ROLE_HOST)

enum 
{
	QT_ERROR_MODULE_SESSION = 65000,
	QT_ERROR_SESSION_INVALID_PDU,
	QT_ERROR_SESSION_NETWORK_ERROR,
	QT_ERROR_SESSION_VIDEO_MODE_CHANGE,
	QT_ERROR_SESSION_USER_JOIN_PENDING,
	QT_ERROR_SESSION_DUMMY_USER_LEAVE, //65005
	QT_ERROR_SESSION_RECONNECT_FAILURE,
	QT_ERROR_SESSION_INVALID_RSPN_FROM_MCC,
	QT_ERROR_SESSION_ILLEAGAL_CLIENT_VERSION,
	QT_ERROR_SESSION_MODEM_USER,
	QT_ERROR_TIMEOUT_WAIT_RSPN_FROM_MCC, //65010
	QT_ERROR_SESSION_DECRYPT_TICKET_FAILED,
	QT_ERROR_SESSION_NOT_SAME_DCID,
	QT_ERROR_SESSION_LOAD_LIMIT,
	QT_ERROR_SESSION_CONF_NOT_EXIST,
	QT_ERROR_SESSION_INCORRECT_CLIENT_TYPE, //65015
	QT_ERROR_SESSION_VOIP_LEAVE_TRANSFER,
	QT_ERROR_SESSION_VIDEO_STOP_BILLING,
	QT_ERROR_SESSION_CAPACITY_LIMIT, //65018
	QT_ERROR_SESSION_BAD_VERSION, 
	QT_ERROR_SESSION_INCONSISTENT_VERSION, 
	QT_ERROR_SESSION_DUPPLICATED_SERVER_ID,
	QT_ERROR_SESSION_DUPPLICATED_SERVER_CONNECTION,
	QT_ERROR_SESSION_CLOSED //65023
};

enum{
	NBR_FILE_READ_SUCCEED = 50000,
	NBR_FILE_READ_FAILED,
	NBR_FILE_READ_OVER
};

enum
{
	QT_SESSION_STATUS_CONNECTED_MCC_SUCCESSFULLY = 55000,	
	QT_SESSION_STATUS_RECV_RESPONSE_FROM_MCC,
	QT_SESSION_STATUS_CONNECTED_MCS_SUCCESSFULLY,
	QT_SESSION_STATUS_SEND_JOIN_SESSION_RQST,
	QT_SESSION_STATUS_SEND_BIND_DATA_CHANNEL_RQST,
	QT_SESSION_STATUS_RECV_BIND_DATA_RSPN
};

typedef DWORD QT_SESSION_OPTION;
enum
{
	//Param. is Pointer to CCmConnectionManager::CType, CTYPE_UDP, etc.
	QT_SESSION_OPTION_DATA_TRANSPORT_TYPE = 1,
		
	//Param. is Pointer to CCmString.
	QT_SESSION_OPTION_ENCRYPTION_KEY,

	// Param. is pointer to QT_SESSION_MODE_TEST_VALUE
	QT_SESSION_OPTION_TEST,

	// Param. is pointer to DWORD
	QT_SESSION_OPTION_BAND_WIDTH,	// = 4

	// Param. is pointer to QT_SESSION_MODE_CONNECTION_VALUE
	QT_SESSION_OPTION_CONNECTION,

	// Param. is pointer to char
	QT_SESSION_OPTION_MCT_GET_MCS_ADDR,

	// Param. is pointer to DWORD
	QT_SESSION_RATE_POLICER_CUR_RATE,

	// Param. is pointer to DWORD
	QT_SESSION_RATE_POLICER_BURST_RATE,	// = 8

	// Param. is pointer to DWORD
	QT_SESSION_RATE_POLICER_LOSS_RATE,

	// Param. is pointer to DWORD
	QT_SESSION_OPTION_MCT_CONN_SELECT,

	// Param. is pointer to char*
	QT_SESSION_OPTION_MCT_MCS_CONFIG,

	//voip filter mode, param: pointer to struct MmSessionFilterMode.
	QT_SESSION_OPTION_FILTER_MODE ,		// = 12
	
	//max active speaker in voip, param: pointer to struct MmSessionMaxActiveChange.
	QT_SESSION_OPTION_FILTER_MAX_SPEAKER,

	// ADDED in the hybrid project, identify the VOIP session mode
	QT_SESSION_OPTION_VOIP_PATTERN,

	// ADDED in T27 task, for support hybrid BO session
	QT_SESSION_OPTION_HYBRID_SEQUENCE_ID,

	// ADDED in T27 task for the location ID in the GDM, its type: char *
	QT_SESSION_OPTION_LOCATION_ID,		// = 16

	// Param is pointer to struct BillingState, added for load control.
	QT_SESSION_OPTION_BILLING_STATE,

	// For MCT SPA test, indicate which SPA will be tested, char *
	QT_SESSION_OPTION_SPA_LOCATION_ID,

	// For MCT SPA test, it is a string, format address:port
	QT_SESSION_OPTION_PROXY_SSL_GATEWAY,

	// For SPA, point to struct MmSessionSPAInfo.
	QT_SESSION_OPTION_SPA_INFO,		// = 20

	// For voice encryption, point to struct MmSessionUDPEncryptionInfo
	QT_SESSION_UDP_ENCRYPTION_INFO,	

	//// Added in MMP_3_4_2, For Version Control
	//// option type: DWORD
	QT_SESSION_OPTION_REQURIED_VERSION, 

	/// option type: DWORD
	QT_SESSION_OPTION_CLIENT_VERSION,

	/// option type: DWORD
	QT_SESSION_OPTION_FEATURES,		// = 24

	QT_SESSION_OPTION_RESERVED_INFO,

	// For voice encryption, point to struct MmSessionUDPEncryptionInfo
	QT_SESSION_UDP_ENCRYPTION_INFO_FROM_OTHER_SERVER,

	QT_SESSION_OPTION_NBR_SUB_MODE,

	//point to address of bool.
	QT_SESSION_OPTION_ENABLE_ASN,		// = 28
	
	// DWORD, for media streaming session BO sequence ID
	QT_SESSION_OPTION_MEDIASTREAMING_BOSEQID,

	//point address of bool
	QT_SESSION_OPTION_SEND_EMPTY_ASN,

	// the QOS control parameter setting option, 
	// the value type MUST BE the pointer to the
	// structure: MmQosControlPara
	QT_SESSION_OPTION_QOS_CONTROL_PARAM
};

typedef DWORD QT_SESSION_MODE_TEST_VALUE;
enum
{
	QT_SESSION_OPTION_TEST_VALUE_NORMAL,
	QT_SESSION_OPTION_TEST_VALUE_MCC_ONLY,
	QT_SESSION_OPTION_TEST_VALUE_MCS_ONLY
};

typedef DWORD QT_SESSION_MODE_CONNECTION_VALUE;
enum
{
	QT_SESSION_OPTION_CONNECTION_VALUE_UDP,
	QT_SESSION_OPTION_CONNECTION_VALUE_TCP,
	QT_SESSION_OPTION_CONNECTION_VALUE_SSL,
	QT_SESSION_OPTION_CONNECTION_VALUE_NORMAL
};

typedef BYTE QT_SESSION_TYPE;
enum 
{ 
	QT_SESSION_TYPE_NONE			= 0, 
	QT_SESSION_TYPE_USER            = 1 << 1,
	QT_SESSION_TYPE_RELAY			= 1 << 2
};


typedef DWORD QT_CLIENT_VERSION_TYPE;
enum
{
	QT_CLIENT_ERROR	= 0x00,
	QT_CLIENT_V1 	= 0x150000,
	QT_CLIENT_VER_MAX
};

BYTE MainVersionToOldValue(BYTE mainVer);
BYTE OldValueToMainVersion(BYTE oldVal);

//// server version
typedef DWORD QT_SERVER_VERSION_TYPE;
enum 
{
	QT_SERVER_VERSION_1_0_0_0 = 0x01000000
};

typedef BYTE QT_SESSION_ENCRYPTION_TYPE;
enum
{
	QT_SESSION_ENCRYPTION_FALSE,
	QT_SESSION_ENCRYPTION_TRUE
};

typedef BYTE QT_SEESION_PRIORITY_TYPE;
enum 
{
	QT_SEESION_PRIORITY_HIGH,
	QT_SEESION_PRIORITY_BELOW_HIGH,
	QT_SEESION_PRIORITY_ABOVE_LOW,
	QT_SEESION_PRIORITY_LOW
};

typedef WORD QT_SESSION_INDICATE_TYPE;
enum
{
	QT_SESSION_IND_NONE 			= 0,
	QT_SESSION_IND_SAMPLE		= 1
};

typedef BYTE QT_KEY_TYPE;
enum
{
	QT_KEY_TYPE_GLOBAL = 0,  //session key for global communication
	QT_KEY_TYPE_LOCAL = 1,   //quantum key for peer-to-peer communication
	QT_KEY_TYPE_PRIVATE = 2  // only personal use
};

enum
{
	QT_NULL = 0,
	QT_QKM,
	QT_QKRS_PARENT,
	QT_QKRS_CHILD,
};

enum
{
	QT_NETWORK_NULL = 0,
	QT_NETWORK_HAVE_QKM,
	QT_NETWORK_NO_QKM,
};

typedef DWORD QT_SESSION_ID;
typedef DWORD QT_SERVER_ID;

typedef BYTE QT_TRANSACTION_STATUS;
enum{
	QT_TRANSACTION_STATUS_NONE = 0,
	QT_TRANSACTION_REQ_SENT = 1,
	QT_TRANSACTION_REQ_RECV = 2,
	QT_TRANSACTION_ACK_SENT = 3,
	QT_TRANSACTION_ACK_RECV = 4,
	QT_TRANSACTION_CONF_SENT = 5,
	QT_TRANSACTION_CONF_RECV = 6,
	QT_TRANSACTION_RELAY_REQ_SENT = 7,
	QT_TRANSACTION_RELAY_REQ_RECV = 8,
	QT_TRANSACTION_RELAY_RESP_SENT = 9,
	QT_TRANSACTION_RELAY_RESP_RECV = 10,
	QT_TRANSACTION_STATUS_INVALID = 11,
	QT_TRANSACTION_DEL_REQ_SENT = 12,
	QT_TRANSACTION_DEL_REQ_RECV = 13,
	QT_TRANSACTION_DEL_ACK_SENT = 14,
	QT_TRANSACTION_DEL_ACK_RECV = 15,
	QT_TRANSACTION_RELAY_DEL_REQ_SENT = 16,
	QT_TRANSACTION_RELAY_DEL_REQ_RECV = 17,
	QT_TRANSACTION_RELAY_DEL_RESP_SENT = 18,
	QT_TRANSACTION_RELAY_DEL_RESP_RECV = 19
};

#define QT_SAFE_DELETE( p )		\
	if ( p )					\
	{							\
		delete p;				\
		p = NULL;				\
	}

#define QT_ARRAY_DELETE( p )	\
	if ( p )					\
	{							\
		delete []p;				\
		p = NULL;				\
	}

#define DEFAULT_NODE_ID_LEN 8
#define DEFAULT_USER_ID_LEN 33
#define DEFAULT_SESSION_ID_LEN 16
#define DEFAULT_SESSION_NAME_LEN 128
#define DEFAULT_USER_NAME_LEN 128
#define DEFAULT_KEY_ID_LEN 16
#define DEFAULT_KEY_LEN 16
#define DEFAULT_DEVICEID_LEN 33
#define MAX_KEY_LEN 64
#define MAX_AUTH_DATA_LEN	1024


#define AUTH_KEY_TYPE_ROOT		"Root_key"
#define AUTH_KEY_TYPE_CLIENT	"Client_key"

#define USER_NAME_ROOT		"Root_user"

#endif	// QT_SESSIONAPIDEFINES_H
