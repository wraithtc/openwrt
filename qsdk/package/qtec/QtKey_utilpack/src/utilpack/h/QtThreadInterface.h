/*------------------------------------------------------*/
/* Thread Interfaces                                    */
/*                                                      */
/* QtThreadInterface.h                                  */
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

#ifndef QTTHREADINTERFACE_H
#define QTTHREADINTERFACE_H

#include "QtDefines.h"

class CQtTimeValue;

//if add event type, append it in the enum
typedef DWORD	EVENT_TYPE;
enum{
	EVENT_UNDEFINED = 0,
	EVENT_TP_BASE = 1,			//1 - 9999 are for TP layer
	EVENT_TCP_ONINPUT,


	EVENT_SESSION_BASE = 10000,	//10000 - 19999 are for session layer

	//for TelephoneDecoder
	EVENT_SESSION_DECODE_START = EVENT_SESSION_BASE + 400,
	EVENT_SESSION_DECODE_STOP  = EVENT_SESSION_BASE + 401,
	EVENT_SESSION_PING_START = EVENT_SESSION_BASE + 402,
	EVENT_SESSION_PING_STOP = EVENT_SESSION_BASE + 403,
	EVENT_SESSION_PING_RESULT = EVENT_SESSION_BASE + 404,
	EVENT_SESSION_PING_DELETE = EVENT_SESSION_BASE + 405,


	ET_TelephoneUserEvent = EVENT_SESSION_BASE + 500,
	ET_CreateDecoderEvent = EVENT_SESSION_BASE + 501,
	ET_TelephonyDataEvent = EVENT_SESSION_BASE + 502,
	ET_TeleDataToUserEvent= EVENT_SESSION_BASE + 503,
	ET_SessionSubInfoEvent= EVENT_SESSION_BASE + 504,

	EVENT_APP_BASE = 20000,		//others are for app layer
};


class QT_OS_EXPORT IQtEvent
{
public:
	virtual QtResult OnEventFire() = 0;

	virtual void OnDestorySelf();
	IQtEvent(EVENT_TYPE EventType = EVENT_UNDEFINED):m_EventType(EventType)
	{
#ifdef WIN32
		m_Tid = ::GetCurrentThreadId();
#else
		m_Tid = ::pthread_self();
#endif // WIN32
	}

protected:
	EVENT_TYPE	m_EventType;		//the event type for tracking
	QT_THREAD_ID	m_Tid;			//the thread id which thread create the event
	virtual ~IQtEvent() { }
};

class QT_OS_EXPORT IQtEventQueue
{
public:
	enum EPriority
	{
		EPRIORITY_HIGH,
		EPRIORITY_NORMAL,
		EPRIORITY_LOW
	};
	
	/// this function could be invoked in the different thread.
	/// like PostMessage() in Win32 API.
	virtual QtResult PostEvent(IQtEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL) = 0;

	/// this function could be invoked in the different thread.
	/// like SendMessage() in Win32 API.
	virtual QtResult SendEvent(IQtEvent *aEvent) = 0;

	/// get the number of pending events.
	virtual DWORD GetPendingEventsCount() = 0;
	
protected:
	virtual ~IQtEventQueue() { }
};

/**
 * @class IQtTimerHandler
 *
 * @brief Provides an abstract interface for handling timer event.
 *
 * Subclasses handle a timer's expiration.
 * 
 */
class QT_OS_EXPORT IQtTimerHandler
{
public:
	/**
	 * Called when timer expires.  <aCurTime> represents the current
	 * time that the <AQtEventHandler> was selected for timeout
	 * dispatching and <aArg> is the asynchronous completion token that
	 * was passed in when <ScheduleTimer> was invoked.
	 * the return value is ignored.
	 */
	virtual void OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg) = 0;

protected:
	virtual ~IQtTimerHandler() { }
};

class QT_OS_EXPORT IQtTimerQueue
{
public:
	/**
	 * this function must be invoked in the own thread.
	 * <aInterval> must be greater than 0.
	 * If success:
	 *    if <aTh> exists in queue, return QT_ERROR_FOUND;
	 *    else return QT_OK;
	 */
	virtual QtResult ScheduleTimer(IQtTimerHandler *aTh, 
					  LPVOID aArg,
					  const CQtTimeValue &aInterval,
					  DWORD aCount) = 0;

	/**
	 * this function must be invoked in the own thread.
	 * If success:
	 *    if <aTh> exists in queue, return QT_OK;
	 *    else return QT_ERROR_NOT_FOUND;
	 */
	virtual QtResult CancelTimer(IQtTimerHandler *aTh) = 0;
	
protected:
	virtual ~IQtTimerQueue() { }
};

#endif // QTTHREADINTERFACE_H
