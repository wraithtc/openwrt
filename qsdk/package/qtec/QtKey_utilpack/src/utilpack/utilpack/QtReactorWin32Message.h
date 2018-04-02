/*------------------------------------------------------*/
/* Reactor using Win32 message system                   */
/*                                                      */
/* QtReactorWin32Message.h                              */
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

#ifndef QTREACTORWIN32MESSAGE_H
#define QTREACTORWIN32MESSAGE_H

#ifndef QT_WIN32
   #error ERROR: only WIN32 supports Win32 Messages!
#endif // QT_WIN32

#include "QtReactorBase.h"


#ifdef QT_ENABLE_CALENDAR_TIMER
  #include "QtTimerQueueCalendar.h"
#endif // QT_ENABLE_CALENDAR_TIMER

class CQtReactorWin32Message : public CQtReactorBase  
{
public:
	CQtReactorWin32Message();
	virtual ~CQtReactorWin32Message();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult NotifyHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask);

	virtual QtResult RunEventLoop();

	virtual QtResult StopEventLoop();
	
	virtual QtResult Close();

protected:
	virtual QtResult OnHandleRegister(QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, AQtEventHandler *aEh);
	virtual void OnHandleRemoved(QT_HANDLE aFd);

	static LRESULT CALLBACK Win32SocketWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND m_hwndNotify;
	UINT m_dwTimerId;

#ifdef QT_ENABLE_CALENDAR_TIMER
	// interface IQtTimerQueue
	virtual QtResult ScheduleTimer(IQtTimerHandler *aTh, 
					  LPVOID aArg,
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	virtual QtResult CancelTimer(IQtTimerHandler *aTh);

	CQtTimerQueueCalendar m_CalendarTimer;
#endif // QT_ENABLE_CALENDAR_TIMER
};

class CQtReactorWin32AsyncSelect : public CQtReactorWin32Message
{
public:
	CQtReactorWin32AsyncSelect();
	virtual ~CQtReactorWin32AsyncSelect();

protected:
	virtual QtResult OnHandleRegister(QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, AQtEventHandler *aEh);
	virtual void OnHandleRemoved(QT_HANDLE aFd);

private:
	QtResult DoAsyncSelect_i(QT_HANDLE aFd, AQtEventHandler::MASK aMask);
};

#endif // !QTREACTORWIN32MESSAGE_H
