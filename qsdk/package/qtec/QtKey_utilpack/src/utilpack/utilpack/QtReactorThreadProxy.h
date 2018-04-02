/*------------------------------------------------------*/
/* Thread proxy wrapper for reactor interface           */
/*                                                      */
/* QtReactorThreadProxy.h                               */
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

#ifndef QTREACTORTHREADPROXY_H
#define QTREACTORTHREADPROXY_H

#include "QtReactorInterface.h"
#include "QtThreadProxyManager.h"

class CQtReactorThreadProxy : public IQtReactor, public CQtThreadProxyBase
{
public:
	CQtReactorThreadProxy(IQtReactor *aReactor, CQtThreadManager::TType aType);
	virtual ~CQtReactorThreadProxy();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult RegisterHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask);

	virtual QtResult RemoveHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask = AQtEventHandler::ALL_EVENTS_MASK);

	virtual QtResult NotifyHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask);

	virtual QtResult RunEventLoop();

	virtual QtResult StopEventLoop();

	virtual QtResult Close();

	// interface IQtTimerQueue
	QtResult ScheduleTimer(IQtTimerHandler *aTh, 
					  LPVOID aArg,
					  const CQtTimeValue &aInterval,
					  DWORD aCount);
	virtual QtResult CancelTimer(IQtTimerHandler *aTh);

	// interface IQtEventQueue
	virtual QtResult PostEvent(IQtEvent *aEvent);

	virtual QtResult SendEvent(IQtEvent *aEvent);

private:
	IQtReactor *m_pReactor;

	class CEventRegisterHandler : public IQtEvent
	{
	public:
		CEventRegisterHandler(IQtReactor *aReactor, AQtEventHandler *aEh, 
			AQtEventHandler::MASK aMask)
			: m_pReactor(aReactor)
			, m_pEh(aEh)
			, m_Mask(aMask)
		{
		}

		virtual ~CEventRegisterHandler();

		virtual QtResult OnEventFire();

	private:
		IQtReactor *m_pReactor;
		AQtEventHandler *m_pEh;
		AQtEventHandler::MASK m_Mask;
	};

	class CEventRemoveHandler : public IQtEvent
	{
	public:
		CEventRemoveHandler(IQtReactor *aReactor, AQtEventHandler *aEh, 
			AQtEventHandler::MASK aMask)
			: m_pReactor(aReactor)
			, m_pEh(aEh)
			, m_Mask(aMask)
		{
		}

		virtual ~CEventRemoveHandler();

		virtual QtResult OnEventFire();

	private:
		IQtReactor *m_pReactor;
		AQtEventHandler *m_pEh;
		AQtEventHandler::MASK m_Mask;
	};
};

#endif // !QTREACTORTHREADPROXY_H
