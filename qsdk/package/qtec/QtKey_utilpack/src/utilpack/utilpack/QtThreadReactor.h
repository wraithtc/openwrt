/*------------------------------------------------------*/
/* One thread composes one reactor                      */
/*                                                      */
/* QtThreadReactor.h                                    */
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

#ifndef QTTHREADREACTOR_H
#define QTTHREADREACTOR_H

#include "QtThreadInterface.h"

class IQtReactor;

class CQtThreadReactor : public AQtThread  
{
	CQtThreadReactor(const CQtThreadReactor &);
	CQtThreadReactor & operator=(const CQtThreadReactor &);
public:
	CQtThreadReactor();
	virtual ~CQtThreadReactor();

	QtResult Init(IQtReactor *aReactor);

	// interface AQtThread
	virtual QtResult Create
		(CQtThreadManager::TType aType, 
		CQtThreadManager::TFlag aFlag = CQtThreadManager::TF_JOINABLE);
	
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);

	virtual void OnThreadInit();
	virtual void OnThreadRun();

	virtual IQtReactor* GetReactor();
	virtual IQtEventQueue* GetEventQueue();
	virtual IQtTimerQueue* GetTimerQueue();

private:
	IQtReactor *m_pReactor;
};

/// used for netwok thread is the same as main thread.
class CQtThreadDummy : public AQtThread  
{
public:
	CQtThreadDummy();
	virtual ~CQtThreadDummy();

	QtResult Init(AQtThread *aThread, CQtThreadManager::TType aType);

	// interface AQtThread
	virtual QtResult Create(
		CQtThreadManager::TType aType, 
		CQtThreadManager::TFlag aFlag = CQtThreadManager::TF_JOINABLE);

	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);

	virtual void OnThreadRun();

	virtual IQtReactor* GetReactor();
	virtual IQtEventQueue* GetEventQueue();
	virtual IQtTimerQueue* GetTimerQueue();

private:
	AQtThread *m_pActualThread;
};

#endif // !QTTHREADREACTOR_H
