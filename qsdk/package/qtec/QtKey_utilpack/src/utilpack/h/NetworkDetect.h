/*------------------------------------------------------*/
/* Check network connetion							    */
/*                                                      */
/* NetworkDetect.h										*/
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

#ifndef __QT_NETWORK_DETECT_H__
#define __QT_NETWORK_DETECT_H__

#include "QtDefines.h"

/*
 *	Check network connetion type;
 *  In: none;
 *  Out:
 *		bModem: if true, it use modem, otherwise use ethern card.
		dwSpeed: e.g if it is 28K modem, then dwSpeed = 28672, the unit is bit.
	Return:
		success: QT_OK;
		failed: other value;
	Note: only support win32 now.
 */

extern "C" QT_OS_EXPORT QtResult QtCheckNetwork(BOOL& bModem, DWORD& dwSpeed);

#endif //!__QT_NETWORK_DETECT_H__
