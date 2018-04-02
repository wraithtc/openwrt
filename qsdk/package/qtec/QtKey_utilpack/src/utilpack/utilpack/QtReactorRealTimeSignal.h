/*------------------------------------------------------*/
/* Reactor using Linux realtime signal                  */
/*                                                      */
/* QtReactorRealTimeSignal.h                            */
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

#ifndef QTREACTORREALTIMESIGNAL_H
#define QTREACTORREALTIMESIGNAL_H

#ifndef QT_LINUX
  #error ERROR: only LINUX supports realtime signal!
#endif // QT_LINUX

#include "QtReactorBase.h"
#include "QtReactorNotifyPipe.h"


#ifdef QT_ENABLE_CALENDAR_TIMER
  #include "QtTimerQueueCalendar.h"
#endif // QT_ENABLE_CALENDAR_TIMER

#include <sys/poll.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/sysctl.h>

class CQtReactorRealTimeSignal : public CQtReactorBase  
{
public:
	CQtReactorRealTimeSignal();
	virtual ~CQtReactorRealTimeSignal();

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

	QtResult ProcessSignalOverFlow();
	QtResult ProcessOneSignalFd(int aFd, long int aBand);
	QtResult ProcessOneSignalQueue(int aVal);

protected:
	int SetProcRtsigMax_i(int aMaxNum);
	QtResult CheckPollIn_i(QT_HANDLE aFd, AQtEventHandler *aEh);
	QtResult Sigqueue_i(int aValue, BOOL aTrace = TRUE);

	// sig values.
	enum { 
		SV_NONE = 0, 
		SV_EVENT, 
		SV_STOP, 
		SV_TIMER 
	};
	static void TimerTickFun(int );

	sigset_t m_Sigset;
	int m_SigNum;
	CQtReactorNotifyPipe m_Notify;
	int m_nPid;
	BOOL m_bSignalOverFlowing;

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

#include <list>

// Buffer rt-signals in user space to increase efficience.
class CQtReactorRealTimeSignalBuffer : public CQtReactorRealTimeSignal
{
public:
	CQtReactorRealTimeSignalBuffer();
	virtual ~CQtReactorRealTimeSignalBuffer();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult RunEventLoop();
	
	virtual QtResult Close();

private: 
	void InsertOneSigalNode_i(siginfo_t &aSigInfo);

public:
	class CSigalNode 
	{
	public:
		CSigalNode()
		{
			::memset(this, 0, sizeof(*this));
		}

		int m_si_code;
		union {
			int m_sival_int;

			struct {
				int m_si_fd;
				long int m_si_band;
				DWORD m_dwReadCount;
			} m_CodeIo;
		};
	};
	// We have to use list because <m_pSigalNodesIndex> will index it.
	typedef std::list<CSigalNode> SigalNodesType;

private:
	SigalNodesType m_SigalNodes;

	CSigalNode **m_pSigalNodesIndex;
	int m_nSigalNodeIndexNumber;
};


// inline functions
inline QtResult CQtReactorRealTimeSignal::Sigqueue_i(int aValue, BOOL aTrace)
{
	sigval svNew;
	svNew.sival_int = aValue;
	if (::sigqueue(m_nPid, m_SigNum, svNew) != 0) {
		if (aTrace) {
			QT_ERROR_TRACE_THIS("CQtReactorRealTimeSignal::Sigqueue_i, sigqueue() failed!"
				" err=" << errno << " aValue=" << aValue);
		}
		else {
			CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
			::printf("CQtReactorRealTimeSignal::Sigqueue_i, sigqueue() failed!"
				" err=%d sec=%ld usec=%ld.\n", errno, tvCur.GetSec(), tvCur.GetUsec());
		}
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
}


#endif // !QTREACTORREALTIMESIGNAL_H
