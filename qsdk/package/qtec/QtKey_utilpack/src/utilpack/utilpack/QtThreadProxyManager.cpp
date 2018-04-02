
#include "QtBase.h"
#include "QtThreadProxyManager.h"


//////////////////////////////////////////////////////////////////////
// class CQtThreadProxyBase
//////////////////////////////////////////////////////////////////////

CQtThreadProxyBase::CQtThreadProxyBase(CQtThreadManager::TType aType)
{
	m_pEventQueue = CQtThreadManager::Instance()->GetThreadEventQueue(aType);
	QT_ASSERTE(m_pEventQueue);
}

CQtThreadProxyBase::CQtThreadProxyBase(AQtThread *aThread)
{
	if (aThread)
		m_pEventQueue = aThread->GetEventQueue();
	else
		m_pEventQueue = NULL;
	QT_ASSERTE(m_pEventQueue);
}

CQtThreadProxyBase::~CQtThreadProxyBase()
{
}

QtResult CQtThreadProxyBase::PostEvent_i(IQtEvent* aEvent)
{
	if (!aEvent)
		return QT_ERROR_INVALID_ARG;
	else if (m_pEventQueue)
		return m_pEventQueue->PostEvent(aEvent);
	else
		return QT_ERROR_UNEXPECTED;
}

QtResult CQtThreadProxyBase::SendEvent_i(IQtEvent* aEvent)
{
	if (!aEvent)
		return QT_ERROR_INVALID_ARG;
	else if (m_pEventQueue)
		return m_pEventQueue->SendEvent(aEvent);
	else
		return QT_ERROR_UNEXPECTED;
}

//////////////////////////////////////////////////////////////////////
// class CQtThreadProxyManager
//////////////////////////////////////////////////////////////////////

CQtThreadProxyManager::CQtThreadProxyManager()
{

}

CQtThreadProxyManager::~CQtThreadProxyManager()
{

}
