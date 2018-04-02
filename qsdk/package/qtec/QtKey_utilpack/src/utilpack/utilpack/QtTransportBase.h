/*------------------------------------------------------*/
/* Tranport base class                                  */
/*                                                      */
/* QtTransportBase.h                                    */
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

#ifndef QTTRANSPORTBASE_H
#define QTTRANSPORTBASE_H

#include "QtReactorInterface.h"
#include "QtConnectionInterface.h"
#include "QtReferenceControl.h"
#include "QtTimerWrapperID.h"
#include "QtTimeValue.h"
#include "QtUtilTemplates.h"

class CQtSocketBase;

class QT_OS_EXPORT CQtTransportBase 
	: public AQtEventHandler
	, public IQtTransport
	, public CQtReferenceControlSingleThreadTimerDelete
{
public:
	CQtTransportBase(IQtReactor *pReactor);
	virtual ~CQtTransportBase();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();

	// interface IQtTransport
	virtual QtResult OpenWithSink(IQtTransportSink *aSink);
	virtual IQtTransportSink* GetSink();
	virtual QtResult Disconnect(QtResult aReason);
	
	// interface AQtEventHandler
	virtual int OnClose(QT_HANDLE aFd, MASK aMask);
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE) = 0;	

#ifdef QT_SUPPORT_QOS
	QtResult SetQos2Socket(QT_HANDLE aSocket);
#endif // QT_SUPPORT_QOS

	QtResult SetTos2Socket(CQtSocketBase &aSocket, LPVOID aArg);

protected:
	// template method for open() and close()
	virtual QtResult Open_t() = 0;
	virtual QtResult Close_t(QtResult aReason) = 0;

	IQtTransportSink *m_pSink;
	IQtReactor *m_pReactor;
};


class CQtReceiveEvent: public IQtEvent
{
	CQtComAutoPtr<CQtTransportBase> m_pTransport;
public:
	CQtReceiveEvent(CQtTransportBase *pTrans): m_pTransport(pTrans){}
	virtual QtResult OnEventFire();
};

#endif // !QTTRANSPORTBASE_H
