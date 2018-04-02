
#include "QtBase.h"
#include "QtReactorThreadProxy.h"

CQtReactorThreadProxy::CQtReactorThreadProxy(IQtReactor *aReactor, 
											 CQtThreadManager::TType aType)
	: IQtReactor(NULL_PROPERTY)
	, CQtThreadProxyBase(aType)
	, m_pReactor(aReactor)
{
	QT_ASSERTE(m_pReactor);
}

CQtReactorThreadProxy::~CQtReactorThreadProxy()
{
}

QtResult CQtReactorThreadProxy::Open()
{
	QT_ASSERTE(!"CQtReactorThreadProxy::Open");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::
RegisterHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	CEventRegisterHandler *pEvent = new CEventRegisterHandler(m_pReactor, aEh, aMask);
	return SendEvent_i(pEvent);
}

QtResult CQtReactorThreadProxy::
RemoveHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	QT_INFO_TRACE_THIS("CQtReactorThreadProxy::RemoveHandler, begin,"
		" aEh=" << aEh << " aMask=" <<aMask);

	CEventRemoveHandler *pEvent = new CEventRemoveHandler(m_pReactor, aEh, aMask);
	QtResult rv = SendEvent_i(pEvent);

	QT_INFO_TRACE_THIS("CQtReactorThreadProxy::RemoveHandler, end.");
	return rv;
}

QtResult CQtReactorThreadProxy::
NotifyHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	QT_ASSERTE(!"CQtReactorThreadProxy::NotifyHandler");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::RunEventLoop()
{
	QT_ASSERTE(!"CQtReactorThreadProxy::RunEventLoop");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::StopEventLoop()
{
	QT_ASSERTE(!"CQtReactorThreadProxy::StopEventLoop");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::Close()
{
	QT_ASSERTE(!"CQtReactorThreadProxy::Close");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::
ScheduleTimer(IQtTimerHandler *aTh, LPVOID aArg,
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	QT_ASSERTE(!"CQtReactorThreadProxy::ScheduleTimer");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::CancelTimer(IQtTimerHandler *aTh)
{
	QT_ASSERTE(!"CQtReactorThreadProxy::CancelTimer");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::PostEvent(IQtEvent* aEvent)
{
	QT_ASSERTE(!"CQtReactorThreadProxy::PostEvent");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtReactorThreadProxy::SendEvent(IQtEvent* aEvent)
{
	QT_ASSERTE(!"CQtReactorThreadProxy::SendEvent");
	return QT_ERROR_NOT_IMPLEMENTED;
}


//////////////////////////////////////////////////////////////////////
// class CQtReactorThreadProxy
//////////////////////////////////////////////////////////////////////

CQtReactorThreadProxy::CEventRegisterHandler::~CEventRegisterHandler()
{
}

QtResult CQtReactorThreadProxy::CEventRegisterHandler::OnEventFire()
{
	QtResult rv = QT_ERROR_NULL_POINTER;
	if (m_pReactor) {
		rv = m_pReactor->RegisterHandler(m_pEh, m_Mask);
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtReactorThreadProxy::CEventRegisterHandler::OnEventFire,"
				" m_pReactor->RegisterHandler() failed!"
				" m_pEh=" << m_pEh << 
				" m_Mask=" << m_Mask << 
				" rv=" << rv);
		}
	}
	return rv;
}


//////////////////////////////////////////////////////////////////////
// class CEventRemoveHandler
//////////////////////////////////////////////////////////////////////

CQtReactorThreadProxy::CEventRemoveHandler::~CEventRemoveHandler()
{
}

QtResult CQtReactorThreadProxy::CEventRemoveHandler::OnEventFire()
{
	QtResult rv = QT_ERROR_NULL_POINTER;
	if (m_pReactor) {
		rv = m_pReactor->RemoveHandler(m_pEh, m_Mask);
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtReactorThreadProxy::CEventRegisterHandler::OnEventFire,"
				" m_pReactor->RemoveHandler() failed!"
				" m_pEh=" << m_pEh << 
				" m_Mask=" << m_Mask << 
				" rv=" << rv);
		}
	}
	QT_INFO_TRACE_THIS("CQtReactorThreadProxy::CEventRegisterHandler::OnEventFire, rv=" << rv);
	return rv;
}
