/*------------------------------------------------------*/
/* Read and write init files                            */
/*                                                      */
/* QtConfigInitFile.h                                   */
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

#ifndef QTCONFIGINITFILE_H
#define QTCONFIGINITFILE_H

#include "QtUtilTemplates.h"

class QT_OS_EXPORT CQtConfigInitFile  
{
public:
	CQtConfigInitFile();
	~CQtConfigInitFile();

	QtResult InitWithFileName(const CQtString &aFileName);

	int GetIntParam(
		const CQtString &aGroup, 
		const CQtString &aKey,
		int aDefault = 0);

	DWORD GetDwordParam(
		const CQtString &aGroup, 
		const CQtString &aKey,
		DWORD aDefault = 0);

	WORD GetWordParam(
		const CQtString &aGroup, 
		const CQtString &aKey,
		WORD aDefault = 0);

	CQtString GetStringParam(
		const CQtString &aGroup, 
		const CQtString &aKey,
		CQtString aDefault = CQtString());

	BOOL GetBoolParam(
		const CQtString &aGroup, 
		const CQtString &aKey,
		BOOL aDefault = FALSE);

	static CQtString GetQtecHomeDir();
	static void SetQtecHomeEnv(const CQtString &aHome);
	static void SetQtecConfigFileName(const CQtString &file);
	
	friend class CQtSingletonT<CQtConfigInitFile>;
};

typedef CQtSingletonT<CQtConfigInitFile> CQtConfigInitFileSingleton;

#endif // !QTCONFIGINITFILE_H
