/*------------------------------------------------------*/
/* Object Manager(residents in main thread)             */
/*                                                      */
/* QtObjectManager.h                                    */
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

#ifndef QTOBJECTMANAGER_H
#define QTOBJECTMANAGER_H

#include "QtDefines.h"
#include "QtMutex.h"

class CQtObjectManager  
{
public:
	CQtObjectManager();
	~CQtObjectManager();

	CQtMutexThreadRecursive& GetSingletonMutex();

private:
	CQtMutexThreadRecursive m_SingletonMutex;

private:
	// = Prevent assignment and initialization.
	typedef CQtObjectManager SelfType;
	void operator = (const SelfType&);
#ifdef QT_MACOS
	CQtObjectManager(const SelfType&);
#else
	SelfType(const SelfType&);
#endif
};

#endif // !QTOBJECTMANAGER_H
