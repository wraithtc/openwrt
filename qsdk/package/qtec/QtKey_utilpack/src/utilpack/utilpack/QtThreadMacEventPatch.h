/*----------------------------------------------------- */
/*                                                      */
/* Webex Application Module For Macintosh Platform 	    */
/*                                                      */
/* QtThreadMacEventPatch.h                              */
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

#ifndef _H_QtThreadMacEventPatch_
#define _H_QtThreadMacEventPatch_
#pragma once

#ifndef MachOSupport
#include <CarbonEvents.h>
#endif	//MachOSupport

#include "QtThread.h"
#include "QtReactorBase.h"

class CQtTimerQueueBase;
class CQtTimeValue;

// WebEx custom event types
const UInt32 kEventClassWebExTpEvent = 'WeTp';

// WebEx custom event kinds
const UInt32 kEventKindWebExTpEvent = 'EKTP';

// WebEx custom event parameter names
const UInt32 kParamNameTpSocketReference = 'nTpS';
const UInt32 kParamTypeTpSocketReference = 'tTpS';

class CQtThreadMacEventPatch
	: public AQtThread
	, public CQtEventQueueUsingMutex
{
public:
	CQtThreadMacEventPatch();
	~CQtThreadMacEventPatch();
	
	// interface AQtThread
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	virtual IQtEventQueue* GetEventQueue();
	virtual IQtTimerQueue* GetTimerQueue();
	
	// interface IQtEventQueue
	virtual QtResult PostEvent(IQtEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);
	
	QtResult ProcessEventQueue();
	QtResult ProcessTimer();
	
private:
	void TpMacSocketMessage();

private:
	CQtTimerQueueBase *m_pTimerQueue;
	BOOL m_bStopped;
	
	CFRunLoopTimerRef	m_nTimer;
	EventTime			m_TimerFrequency ;
	
	EventHandlerUPP		m_handlerUPP;
	EventHandlerRef		m_TpEventHandlerRef;		// cache this and dispose of at app quit	
	
	CFStringRef			m_cfMMPObserveName;
//	CFStringRef			m_cfMMPKeyName;
	DWORD				m_nObserverID;
};

#endif //_H_QtThreadMacEventPatch_
