/*------------------------------------------------------*/
/* Classes of EventHandler and Reactor                  */
/*                                                      */
/* QtReactorInterface.h                                 */
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

#ifndef QTREACTORINTERFACE_H
#define QTREACTORINTERFACE_H

#ifndef QT_USE_REACTOR_SELECT
//#define QT_USE_REACTOR_SELECT
#endif // QT_USE_REACTOR_SELECT

#if defined (QT_WIN32)
//  #define QT_ENABLE_CALENDAR_TIMER
#elif defined (QT_LINUX)
  #define QT_ENABLE_CALENDAR_TIMER
#endif // QT_WIN32

#include "QtDefines.h"
#include "QtThreadInterface.h"

class CQtTimeValue;
class CQtInetAddr;

/**
 * @class AQtEventHandler
 *
 * @brief Provides an abstract interface for handling various types of I/O events.
 *
 * Subclasses read/write input/output on an I/O descriptor(implemented),
 * handle an exception raised on an I/O descriptor(not implemented yet)
 * 
 */
class QT_OS_EXPORT AQtEventHandler
{
public:
	typedef long MASK;
	enum {
		NULL_MASK = 0,
		ACCEPT_MASK = (1 << 0),
		CONNECT_MASK = (1 << 1),
		READ_MASK = (1 << 2),
		WRITE_MASK = (1 << 3),
		EXCEPT_MASK = (1 << 4),
		TIMER_MASK = (1 << 5),
		ALL_EVENTS_MASK = READ_MASK |
                      WRITE_MASK |
                      EXCEPT_MASK |
                      ACCEPT_MASK |
                      CONNECT_MASK |
                      TIMER_MASK,
		SHOULD_CALL = (1 << 6),
		CLOSE_MASK = (1 << 7),
		EVENTQUEUE_MASK = (1 << 8),
		UDP_LINK_MASK = (1 << 9)
	};

	virtual QT_HANDLE GetHandle() const ;
	
	/// Called when input events occur (e.g., data is ready).
	/// OnClose() will be callbacked if return -1.
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	/// Called when output events are possible (e.g., when flow control
	/// abates or non-blocking connection completes).
	/// OnClose() will be callbacked if return -1.
	virtual int OnOutput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	/// Called when an exceptional events occur (e.g., OOB data).
	/// OnClose() will be callbacked if return -1.
	/// Not implemented yet.
	virtual int OnException(QT_HANDLE aFd = QT_INVALID_HANDLE);

	/**
	 * Called when a <On*()> method returns -1 or when the
	 * <RemoveHandler> method is called on an <CReactor>.  The
	 * <aMask> indicates which event has triggered the
	 * <HandleClose> method callback on a particular <aFd>.
	 */
	virtual int OnClose(QT_HANDLE aFd, MASK aMask);
	
	virtual ~AQtEventHandler();
};

/**
 * @class IQtReactor
 *
 * @brief An abstract class for implementing the Reactor Pattern.
 * 
 */
class QT_OS_EXPORT IQtReactor : public IQtEventQueue, public IQtTimerQueue
{
public:
	/// Initialization.
	virtual QtResult Open() = 0;

	/**
	 * Register <aEh> with <aMask>.  The handle will always
	 * come from <GetHandle> on the <aEh>.
	 * If success:
	 *    if <aEh> is registered, return QT_ERROR_FOUND;
	 *    else return QT_OK;
	 */
	virtual QtResult RegisterHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask) = 0;

	/**
	 * Removes <aEh> according to <aMask>. 
	 * If success:
	 *    if <aEh> is registered
	 *       If <aMask> equals or greater than that registered, return QT_OK;
	 *       else return QT_ERROR_FOUND;
	 *    else return QT_ERROR_NOT_FOUND;
	 */
	virtual QtResult RemoveHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask = AQtEventHandler::ALL_EVENTS_MASK) = 0;

	virtual QtResult NotifyHandler(
		AQtEventHandler *aEh,
		AQtEventHandler::MASK aMask) = 0;

	virtual QtResult RunEventLoop() = 0;

	/// this function can be invoked in the different thread.
	virtual QtResult StopEventLoop() = 0;

	/// Close down and release all resources.
	virtual QtResult Close() = 0;

	typedef long PROPERTY;
	enum {
		NULL_PROPERTY = 0,
		SEND_REGISTER_PROPERTY = (1 << 0)
	};

	PROPERTY GetProperty()
	{
		return m_Property;
	}

	IQtReactor(PROPERTY aProperty) 
		: m_Property(aProperty)
	{
	}

	virtual ~IQtReactor();

protected:
	PROPERTY m_Property;
};

/**
 * @class AQtConnectorInternal
 *
 * @brief for internal connector, implement TCP, UDP, HTTP, SSL, etc.
 *
 */
class QT_OS_EXPORT AQtConnectorInternal
{
public:
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL) = 0;
	virtual int Close(QtResult aReason) = 0;

	virtual ~AQtConnectorInternal() { }
};

#endif // !QTREACTORINTERFACE_H
