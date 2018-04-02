/*------------------------------------------------------*/
/* Common base (for UtilTp)                             */
/*                                                      */
/* QtBase.h                                             */
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

#ifndef QTBASE_H
#define QTBASE_H

//#define QT_SUPPORT_QOS

#include "QtDefines.h"
#include "QtError.h"
#include "QtDebug.h"
#include "QtStdCpp.h"
#include "QtReferenceControl.h"
#include "QtMutex.h"
#include "QtThreadManager.h"
#include "QtThread.h"
#include "QtUtilClasses.h"
#include "QtErrorNetwork.h"
#include "QtTimeValue.h"

extern "C"
{
#if defined QT_WIN32 || defined QT_PORT_CLIENT 
	QT_OS_EXPORT const char* QT_strcaserstr(
		const char *big, 
		const char *little);

	QT_OS_EXPORT void QT_Base64Decode(
		const char *bufcoded, 
		CQtString &sbDest);

	QT_OS_EXPORT void QT_Base64Encode(
		const unsigned char *bufin, 
		unsigned long nbytes, 
		CQtString &sbDest);
#endif

}

template<typename T>
QtResult xtoa_wbx(T value, char *pOutBuff, int nOutBuffLen)
{
	if(nOutBuffLen <= 2 || !pOutBuff)  //have no enough space to store it
		return QT_ERROR_FAILURE;
	
	BOOL isLessZero = FALSE;
	if(value <= (T)0) 
	{
		isLessZero =  TRUE;
		value = -value;
	}
	pOutBuff[nOutBuffLen - 1] = 0;
	for(int i = nOutBuffLen - 2; i >= 0; --i)
	{
		pOutBuff[i] = value % 10 + char('0');
		value /= 10;
		if(value == 0) //over
		{
			if(isLessZero)
				pOutBuff[--i] = '-';
			memmove(pOutBuff, pOutBuff + i, nOutBuffLen - i);
			return QT_OK;
		}
	}
	return QT_ERROR_FAILURE;
}
#ifdef QT_WIN32

extern HANDLE g_hInstDll;
HANDLE GetTPDllHandle();

//Get Current module directory
QT_OS_EXPORT CQtString GetModuelPath();

#endif
#endif // !QTBASE_H
