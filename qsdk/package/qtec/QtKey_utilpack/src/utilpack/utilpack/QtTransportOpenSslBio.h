/*------------------------------------------------------*/
/* SSL BIO for transport                                */
/*                                                      */
/* QtTransportOpenSslBio.h                              */
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

#ifndef QTTRANSPORTOPENSSLBIO_H
#define QTTRANSPORTOPENSSLBIO_H

#include <openssl/bio.h>

class CQtTransportOpenSsl;

class QT_OS_EXPORT CQtTransportOpenSslBio  
{
public:
	enum { BIO_TYPE = (33 | BIO_TYPE_SOURCE_SINK) };
	
	static BIO* CreateOne(CQtTransportOpenSsl *aTrpt);
	
	static int  BIO_write (BIO *aBIO, const char *aBuf, int aLen);
	static int  BIO_read  (BIO *aBIO, char *aBuf, int aLen);
	static int  BIO_puts  (BIO *aBIO, const char *aStr);
	static long BIO_ctrl  (BIO *aBIO, int aQtd, long aNum, void *aPtr);
	static int  BIO_new   (BIO *aBIO);
	static int  BIO_free  (BIO *aBIO);
};

#endif // !QTTRANSPORTOPENSSLBIO_H
