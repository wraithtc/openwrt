/*------------------------------------------------------*/
/* Timer queue base class                               */
/*                                                      */
/* QtTimerQueueBase.h                                   */
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

#ifndef QTTIMERQUEUEBASE_H
#define QTTIMERQUEUEBASE_H

#include "QtMutex.h"
#include "QtTimeValue.h"
#include "QtThreadInterface.h"

class IQtObserver;

class QT_OS_EXPORT CQtTimerQueueBase : public IQtTimerQueue
{
public:
	struct CNode
	{
		CNode(IQtTimerHandler *aEh = NULL, LPVOID aToken = NULL) 
			: m_pEh(aEh), m_pToken(aToken), m_dwCount(0) 
		{ }

		IQtTimerHandler *m_pEh;
		LPVOID m_pToken;
		CQtTimeValue m_tvExpired;
		CQtTimeValue m_tvInterval;
		DWORD m_dwCount;
	};
	
	CQtTimerQueueBase(IQtObserver *aObserver);
	virtual ~CQtTimerQueueBase();

#if 1
	// interface IQtTimerQueue
	virtual QtResult ScheduleTimer(IQtTimerHandler *aEh, 
					  LPVOID aToken, 
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	virtual QtResult CancelTimer(IQtTimerHandler *aEh);
#else
	/// if <aInterval> equals zero, the <aCount> must be 1.
	/// If success:
	///    if <aEh> exists in queue, return 1;
	///    else return 0;
	/// else
	///    return -1;
	int ScheduleTimer(IQtTimerHandler *aEh, 
					  LPVOID aToken, 
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	/// If success:
	///    if <aEh> exists in queue, return 0;
	///    else return 1;
	/// else
	///    return -1;
	int CancelTimer(IQtTimerHandler *aEh);
#endif

	/// if the queue is empty, return CQtTimeValue::s_tvMax.
	CQtTimeValue GetEarliestTime();

	/// return the number of timer expired.
	/// and fill <aRemainTime> with the sub-value of earliest time and current time,
	/// if no timer items in the queue, fill <aRemainTime> with <CQtTimeValue::s_tvMax>.
	int CheckExpire(CQtTimeValue *aRemainTime = NULL);

protected:
	/// the sub-classes of CQtTimerQueueBase always use STL contains that 
	/// we just let them manage the memery allocation of CNode
	
	/// the following motheds are all called after locked
	virtual int PushNode_l(const CNode &aPushNode) = 0;
	virtual int EraseNode_l(IQtTimerHandler *aEh) = 0;
	virtual int RePushNode_l(const CNode &aPushNode) = 0;
	virtual int PopFirstNode_l(CNode &aPopNode) = 0;
	virtual int GetEarliestTime_l(CQtTimeValue &aEarliest) const = 0;

	typedef CQtMutexNullSingleThread MutexType;
	MutexType m_Mutex;
	IQtObserver *m_pObserver;
};

#endif // !QTTIMERQUEUEBASE_H
