/*------------------------------------------------------*/
/* Reactor using Linux epoll                            */
/*                                                      */
/* QtReactorEpoll.h                                     */
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

#ifndef QTREACTOREPOLL_H
#define QTREACTOREPOLL_H

#if !defined (QT_LINUX) && !defined (QT_ENABLE_EPOLL)
  #error ERROR: Supports epoll only on LINUX!
#endif // !QT_LINUX && !QT_ENABLE_EPOLL

#include "QtReactorBase.h"
#include "QtReactorNotifyPipe.h"

#ifdef QT_ENABLE_CALENDAR_TIMER
  #include "QtTimerQueueCalendar.h"
#endif // QT_ENABLE_CALENDAR_TIMER

#include <sys/epoll.h>

class CQtReactorEpoll : public CQtReactorBase 
{
public:
	CQtReactorEpoll();
	virtual ~CQtReactorEpoll();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult NotifyHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask);

	virtual QtResult RunEventLoop();

	virtual QtResult StopEventLoop();
	
	virtual QtResult Close();

	virtual QtResult RegisterHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask);

	virtual QtResult RemoveHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask = AQtEventHandler::ALL_EVENTS_MASK);

protected:
	virtual QtResult OnHandleRegister(QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, AQtEventHandler *aEh);
	virtual void OnHandleRemoved(QT_HANDLE aFd);

	QtResult DoEpollCtl_i(QT_HANDLE aFd, AQtEventHandler::MASK aMask, int aOperation);
	
protected:
	QT_HANDLE m_fdEpoll;
	struct epoll_event *m_pEvents;
	CQtReactorNotifyPipe m_Notify;
	int m_nEventsBeginIndex;
	int m_nEventsEndIndex;

#ifdef QT_ENABLE_CALENDAR_TIMER
	// interface IQtTimerQueue
	virtual QtResult ScheduleTimer(IQtTimerHandler *aTh, 
					  LPVOID aArg,
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	virtual QtResult CancelTimer(IQtTimerHandler *aTh);

	CQtTimerQueueCalendar m_CalendarTimer;
	DWORD m_dwWallTimerJiffies;
#endif // QT_ENABLE_CALENDAR_TIMER
};

#endif // !QTREACTOREPOLL_H
