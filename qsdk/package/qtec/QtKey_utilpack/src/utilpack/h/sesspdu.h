/*------------------------------------------------------*/
/* Session Init PDU                                     */
/*                                                      */
/* sesspdu.h                                            */
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

#ifndef __SESS_PDU_H
#define __SESS_PDU_H

#include "QtDefines.h"
#include "QtDebug.h"
#include "QtMessageBlock.h"
#include "QtByteStream.h"
#include "cmcrypto.h"

#define NET_SESS_INIT_PDU_HAVE_EKEY			0x1
#define NET_SESS_INIT_PDU_HAVE_UPPER_DATA	0x2
#define NET_SESS_INIT_PDU_HAVE_RAW_KEY		0x4

#define MAX_EKEY_LEN	65536

class CSessionInitPdu 
{
public:
	CSessionInitPdu(
		IQtCrypto* pCrypto,
		DWORD dwUserId = 0, 
		DWORD dwSiteId = 0,
		DWORD dwConfId = 0, 
		BYTE byConfType = 0,
		WORD wUpperLen = 0, 
		BYTE *pUpper = NULL, 
		WORD wKeyLen = 0, 
		BYTE *pKey = NULL, 
		char *pCert = NULL);

	~CSessionInitPdu();

	BOOL Encode(CQtMessageBlock& mb);
	BOOL Decode(CQtMessageBlock& mb);
	DWORD GetLen(void);

	DWORD GetUserId();
	DWORD GetSiteId();
	DWORD GetConfID();
	BYTE  GetConfType();
	BYTE *GeteKey();
	WORD  GeteKeyLen();
	BYTE *GetKey();
	WORD  GetKeyLen();
	BYTE *GetUpperData();
	WORD  GetUpperDataLen(); 

private:
	DWORD m_dwUserId;
	DWORD m_dwSiteId;
	DWORD m_dwConfId;
	BYTE  m_byConfType;
	BYTE  m_byFlag;

	WORD  m_wEkeyLen;
	BYTE  *m_pEKey;
	BOOL  m_bEKeyAllocByCrypto;

	WORD  m_wKeyLen;
	BYTE  *m_pKey;
	BOOL  m_bKeyAllocByCrypto;

	WORD  m_wUpperLen;
	BYTE  *m_pUpper;
	char  *m_pCert;

	CQtComAutoPtr<IQtCrypto> m_pCrypto;
};
 
#endif //__SESS_PDU_H

