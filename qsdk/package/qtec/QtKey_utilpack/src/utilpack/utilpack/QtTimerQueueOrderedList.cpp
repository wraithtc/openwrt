
#include "QtBase.h"
#include "QtTimerQueueOrderedList.h"


CQtTimerQueueOrderedList::CQtTimerQueueOrderedList(IQtObserver *aObserver)
	: CQtTimerQueueBase(aObserver)
{
}

CQtTimerQueueOrderedList::~CQtTimerQueueOrderedList()
{
}

int CQtTimerQueueOrderedList::EraseNode_l(IQtTimerHandler *aEh)
{
	NodesType::iterator iter = m_Nodes.begin();
	while (iter != m_Nodes.end()) {
		if ((*iter).m_pEh == aEh) {
			m_Nodes.erase(iter);
			return 0;
		}
		else
			++iter;
	}
	return 1;
}

int CQtTimerQueueOrderedList::PopFirstNode_l(CNode &aPopNode)
{
	QT_ASSERTE_RETURN(!m_Nodes.empty(), -1);

	aPopNode = m_Nodes.front();
	m_Nodes.pop_front();
	return 0;
}

int CQtTimerQueueOrderedList::RePushNode_l(const CNode &aPushNode)
{
	NodesType::iterator iter = m_Nodes.begin();
	for ( ; iter != m_Nodes.end(); ++iter ) {
		if ((*iter).m_tvExpired >= aPushNode.m_tvExpired) {
			m_Nodes.insert(iter, aPushNode);
			break;
		}
	}
	if (iter == m_Nodes.end())
		m_Nodes.insert(iter, aPushNode);

//	EnsureSorted();
	return 0;
}

int CQtTimerQueueOrderedList::GetEarliestTime_l(CQtTimeValue &aEarliest) const
{
	if (!m_Nodes.empty()) {
		aEarliest = m_Nodes.front().m_tvExpired;
		return 0;
	}
	else
		return -1;
}

int CQtTimerQueueOrderedList::EnsureSorted()
{
#ifdef QT_DEBUG
	if (m_Nodes.size() <= 1)
		return 0;
	CQtTimeValue tvMin = (*m_Nodes.begin()).m_tvExpired;
	NodesType::iterator iter1 = m_Nodes.begin();
	for ( ++iter1; iter1 != m_Nodes.end(); ++iter1 ) {
		QT_ASSERTE_RETURN((*iter1).m_tvExpired >= tvMin, -1);
		tvMin = (*iter1).m_tvExpired;
	}
#endif // QT_DEBUG
	return 0;
}

int CQtTimerQueueOrderedList::PushNode_l(const CNode &aPushNode)
{
	BOOL bFoundEqual = FALSE;
	BOOL bInserted = FALSE;
	NodesType::iterator iter = m_Nodes.begin();
	while (iter != m_Nodes.end()) {
		if ((*iter).m_pEh == aPushNode.m_pEh) {
			QT_ASSERTE(!bFoundEqual);
			iter = m_Nodes.erase(iter);
			bFoundEqual = TRUE;
			if (bInserted || iter == m_Nodes.end())
				break;
		}

		if (!bInserted && (*iter).m_tvExpired >= aPushNode.m_tvExpired) {
			iter = m_Nodes.insert(iter, aPushNode);
			++iter;
			bInserted = TRUE;
			if (bFoundEqual)
				break;
		}
		++iter;
	}

#ifdef QT_DEBUG
	if (iter != m_Nodes.end())
		QT_ASSERTE(bInserted && bFoundEqual);
#endif // QT_DEBUG

	if (!bInserted)
		m_Nodes.push_back(aPushNode);

	EnsureSorted();
	return bFoundEqual ? 1 : 0;
}
