/*------------------------------------------------------*/
/* Timer queue implemented by ordered list              */
/*                                                      */
/* QtTimerQueueOrderedList.h                            */
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

#ifndef QTTIMERQUEUEORDEREDLIST_H
#define QTTIMERQUEUEORDEREDLIST_H

#include "QtTimerQueueBase.h"
#include <list>

class CQtTimerQueueOrderedList : public CQtTimerQueueBase  
{
public:
	CQtTimerQueueOrderedList(IQtObserver *aObserver);
	virtual ~CQtTimerQueueOrderedList();

protected:
	virtual int PushNode_l(const CNode &aPushNode);
	virtual int EraseNode_l(IQtTimerHandler *aEh);
	virtual int RePushNode_l(const CNode &aPushNode);
	virtual int PopFirstNode_l(CNode &aPopNode);
	virtual int GetEarliestTime_l(CQtTimeValue &aEarliest) const;

private:
	int EnsureSorted();

	typedef std::list<CNode> NodesType;
	NodesType m_Nodes;
};

#endif // !QTTIMERQUEUEORDEREDLIST_H
