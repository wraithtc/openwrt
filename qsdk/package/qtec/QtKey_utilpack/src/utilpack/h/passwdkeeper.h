/*------------------------------------------------------*/
/* passwd encrypt/decrypt implement for client          */
/*                                                      */
/*    passwdkeeper.h                                    */
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

#ifndef __QTEC_PASSWD_KEEPER_H__
#define __QTEC_PASSWD_KEEPER_H__

#include "imutilbase.h"

extern "C"
{
	/*
	Description: EncryptPasswd By AES algorithm.
	input: userid & passwd
	output: encryptedpasswd, the out put data will be
 			in length of multiple of 16. So, caller needs to leave some room. e.g 1024 buffer is enough for encrypted passwd
	return: 0 or -1, means there is error, if success, return the length of output data in multiple of 16.		
	*/

	LIBUTIL_EXPORT int EncryptPasswd(DWORD userid, const char* passwd, unsigned char* encryptedpasswd);

	/*
	Description: Decrypt Passwd
	input : userid, encryptedpasswd, and length as encryptedpasswd length
	return: NULL: means error, else success, user don't need delete/free it.
	*/
	LIBUTIL_EXPORT char* DecryptPasswd(DWORD userid, const LPBYTE encryptedpasswd, DWORD length);
}

#endif //!__QTEC_PASSWD_KEEPER_H__

