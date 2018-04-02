/*------------------------------------------------------*/
/* A simply wrapper for timer                           */
/*                                                      */
/* QtTimerWrapperID.h                                   */
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

#ifndef QTTIMERWRAPPERID_H
#define QTTIMERWRAPPERID_H

#include "QtBase.h"
#include "QtReactorInterface.h"

class CTimeValue;
class CQtTimerWrapperID;

class QT_OS_EXPORT CQtTimerWrapperIDSink
{
public:
	virtual void OnTimer(CQtTimerWrapperID* aId) = 0;

protected:
	virtual ~CQtTimerWrapperIDSink();
};

class QT_OS_EXPORT CQtTimerWrapperID : public IQtTimerHandler
{
public:
	CQtTimerWrapperID();
	virtual ~CQtTimerWrapperID();

	/// Schedule an timer that will expire after <aInterval> for <aCount> times. 
	/// if <aCount> is 0, schedule infinite times.
	QtResult Schedule(
		CQtTimerWrapperIDSink* aSink,
		const CQtTimeValue &aInterval,
		DWORD aCount = 0);

	/// Cancel the timer.
	QtResult Cancel();

protected:
	virtual void OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg);

private:
	BOOL m_bScheduled;
	IQtTimerQueue *m_pTimerQueue;
};

#endif // !QTTIMERWRAPPERID_H
