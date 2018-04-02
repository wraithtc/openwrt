/*------------------------------------------------------*/
/* Thread proxy manager                                 */
/*                                                      */
/* QtThreadProxyManager.h                               */
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

#ifndef QTTHREADPROXYMANAGER_H
#define QTTHREADPROXYMANAGER_H

#include "QtThreadManager.h"
#include "QtThreadInterface.h"

class QT_OS_EXPORT CQtThreadProxyBase
{
public:
	explicit CQtThreadProxyBase(CQtThreadManager::TType aType);
	explicit CQtThreadProxyBase(AQtThread *aThread);
	~CQtThreadProxyBase();

	QtResult PostEvent_i(IQtEvent* aEvent);
	QtResult SendEvent_i(IQtEvent* aEvent);

protected:
	IQtEventQueue *m_pEventQueue;
};

class QT_OS_EXPORT CQtThreadProxyManager  
{
public:
	~CQtThreadProxyManager();

	CQtThreadProxyManager* Instance();

private:
	CQtThreadProxyManager();
};

#endif // !QTTHREADPROXYMANAGER_H
