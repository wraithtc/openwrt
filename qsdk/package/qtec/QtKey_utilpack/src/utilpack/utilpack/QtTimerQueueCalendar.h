/*------------------------------------------------------*/
/* Calendar timer queue                                 */
/*                                                      */
/* QtTimerQueueCalendar.h                               */
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

#ifndef QTTIMERQUEUECALENDAR_H
#define QTTIMERQUEUECALENDAR_H

#include "QtStdCpp.h"
#include "QtThreadInterface.h"
#include "QtUtilClasses.h"
#include <map>

class IQtEventQueue;

template <class ValueType>
class CQtTimerQueueCalendarSlotT
{
public:
	CQtTimerQueueCalendarSlotT *m_pNext;
	ValueType m_Value;
};

/// Implement calendar timer. 
/// The timer callback is not precision, it's decided by <aSlotInterval>.
class CQtTimerQueueCalendar : public IQtTimerQueue, public IQtEvent
{
	CQtTimerQueueCalendar(const CQtTimerQueueCalendar &);
	CQtTimerQueueCalendar & operator=(const CQtTimerQueueCalendar &);
public:
	// <aSlotInterval> is the time value in ms of one slot,
	// <aMaxTime> is total time value in ms of all slots.
	// <aEq>  is used to notify when <aInterval> of ScheduleTimer() equals 0.
	CQtTimerQueueCalendar(
		DWORD aSlotInterval, 
		DWORD aMaxTime, 
		IQtEventQueue *aEq);

	virtual ~CQtTimerQueueCalendar();

	// interface IQtTimerQueue
	virtual QtResult ScheduleTimer(IQtTimerHandler *aEh, 
					  LPVOID aToken, 
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	virtual QtResult CancelTimer(IQtTimerHandler *aEh);

	// interface IQtEvent
	// <aInterval> of ScheduleTimer() equals 0.
	virtual QtResult OnEventFire();
	virtual void OnDestorySelf();

	// timer tick every <aSlotInterval>.
	void TimerTick();

private:
	typedef IQtTimerHandler* HandlerType;
	struct ValueType
	{
		ValueType(HandlerType aEh, LPVOID aToken, 
				  const CQtTimeValue &aInterval, DWORD aCount) 
			: m_pHanler(aEh), m_pToken(aToken)
			, m_tvInterval(aInterval), m_dwCount(aCount) 
		{ }

		bool operator == (const ValueType &aRight) const
		{
			return m_pHanler == aRight.m_pHanler;
		}

		HandlerType m_pHanler;
		LPVOID m_pToken;
		CQtTimeValue m_tvInterval;
		DWORD m_dwCount;
	} ;
	
	typedef CQtTimerQueueCalendarSlotT<ValueType> SlotType;
	typedef std::allocator<SlotType> AllocType;
	typedef std::map<HandlerType, DWORD> HanlersType;

private:
	SlotType* NewSlot_i(const ValueType &aValue)
	{
		SlotType *pNew = m_Alloc.allocate(1, NULL);
		if (pNew) {
			pNew->m_pNext = NULL;
			new (&pNew->m_Value) ValueType(aValue);
		}
		return pNew;
	}
	
	void DeleteSlot_i(SlotType *aSlot)
	{
		aSlot->m_Value.~ValueType();
		m_Alloc.deallocate(aSlot, 1);
	}
/*
	SlotType* PopBack_i(SlotType *aFirst)
	{
		if (!aFirst)
			return NULL;

		SlotType *pCur = aFirst;
		SlotType *pNext = pCur->m_pNext;
		while (pNext) {
			if (!pNext->m_pNext) {
				pCur->m_pNext = NULL;
				return pNext;
			}
			else {
				pCur = pNext;
				pNext = pNext->m_pNext;
			}
		}
		return pCur;
	}
*/
	SlotType* RemoveUniqueHandler_i(const HandlerType &aHanler);
	SlotType* RemoveUniqueSlot_i(
		SlotType *&aFirst, 
		const HandlerType &aHanler);

	void InsertUnique_i(const CQtTimeValue &aInterval, SlotType *aInsert);
	
public:
	CQtEnsureSingleThread m_Est;

private:
	DWORD m_dwInterval;
	SlotType **m_ppSlots;
	DWORD m_dwMaxSlotNumber;
	DWORD m_dwCurrentSlot;
	AllocType m_Alloc;
	IQtEventQueue *m_pEventQueue;
	SlotType *m_pEventSlot;
	HanlersType m_Hanlers;
};

#endif // !QTTIMERQUEUECALENDAR_H
