/*------------------------------------------------------*/
/* Asynchronous DNS resolve                             */
/*                                                      */
/* QtDnsManager.h                                       */
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

#ifndef QTDNSMANAGER_H
#define QTDNSMANAGER_H

#if defined(QT_WIN32) && !defined(QT_WIN32_DISABLE_AsyncGetHostByName)
  #define QT_WIN32_ENABLE_AsyncGetHostByName
#endif // QT_WIN32 && !QT_WIN32_DISANABLE_AsyncGetHostByName

#include "QtStdCpp.h"
#include "QtReferenceControl.h"
#include "QtUtilTemplates.h"
#include "QtMutex.h"
#include "QtThreadInterface.h"
#include "QtObserver.h"
#include "QtTimerWrapperID.h"
#include <map>
#include <vector>
#include <list>

class AQtThread;
class IQtObserver;
class CQtDnsRecord;

class QT_OS_EXPORT CQtDnsManager : public IQtEvent, public CQtTimerWrapperIDSink
{
public:
	static CQtDnsManager* Instance();

	/**
     * gets ip address from cache or kicks off an asynchronous host lookup.
     *
     * @param aRecord
     *        return DNS record corresponding to the given hostname.
     * @param aHostName
     *        the hostname or IP-address-literal to resolve.
     * @param aObserver
     *        the listener to be notified when the result is available.
     * @param aBypassCache
     *        if true, the internal DNS lookup cache will be bypassed.
     * @param aThreadListener
     *        optional parameter (may be null).  if non-null, this parameter
     *        specifies the AQtThread on which the listener's
     *        OnObserve should be called.  however, if this parameter is
     *        null, then OnObserve will be called on current thread.
     *
     * @return 
     *        if QT_OK, <aRecord> is filled with corresponding record.
     *        else if QT_ERROR_WOULD_BLOCK, <aObserver> will be callback late only  
     *   once. You should call CancelResolve() before callback.
     *        else resolve hostname failed.
     */
	QtResult AsyncResolve(
		CQtDnsRecord *&aRecord,
		const CQtString &aHostName,
		IQtObserver *aObserver = NULL,
		BOOL aBypassCache = FALSE,
		AQtThread *aThreadListener = NULL);

	/// <aObserver> will not notified after calling this function.
	QtResult CancelResolve(IQtObserver *aObserver);

	/// clear the <aHostName> in the cache and resolve it again.
	/// return QT_ERROR_WOULD_BLOCK: is relsoving.
	/// return QT_ERROR_FAILURE: relsove failed.
	QtResult RefreshHost(const CQtString &aHostName);

	/// close and release any resoruce.
	QtResult Shutdown();

	/**
     * gets ip address from cache or resolve it at once,
	 * the current thread will be blocking when resolving.
     *
     * @param aRecord
     *        return DNS record corresponding to the given hostname.
     * @param aHostName
     *        the hostname or IP-address-literal to resolve.
     * @param aBypassCache
     *        if true, the internal DNS lookup cache will be bypassed.
     *
     * @return 
     *        if QT_OK, <aRecord> is filled with corresponding record.
     *        else resolve hostname failed.
     */
	QtResult SyncResolve(
		CQtDnsRecord *&aRecord, 
		const CQtString &aHostName,
		BOOL aBypassCache = FALSE);

	QtResult GetLocalIps(CQtDnsRecord *&aRecord);

protected:
	// interface IQtEvent
	virtual QtResult OnEventFire();
	virtual void OnDestorySelf();

	// interface CQtTimerWrapperIDSink
	virtual void OnTimer(CQtTimerWrapperID* aId);

private:
	CQtDnsManager();
	virtual ~CQtDnsManager();
	friend class CQtSingletonT<CQtDnsManager>;

	int BeginResolve_l(CQtDnsRecord *aRecord);
	int DoGetHostByName_l(CQtDnsRecord *aRecord);
	QtResult TryAddObserver_l(IQtObserver *aObserver, AQtThread *aThreadListener, const CQtString &aHostName);
	QtResult Resolved_l(CQtDnsRecord *aRecord, int aError, BOOL aCallback = TRUE);
	QtResult DoCallback_l(int aError, const CQtString &aHostName);
	QtResult FindInCache_l(CQtDnsRecord *&aRecord, const CQtString &aHostName);
	void CopyHostent_i(CQtDnsRecord *aRecord, struct hostent *aHostent);

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	QtResult CreateDnsWindow();
	int DoAsyncHostByName_l(CQtDnsRecord *aRecord);
#endif
	QtResult SpawnDnsThread_l();
//#endif // QT_WIN32_ENABLE_AsyncGetHostByName

private:
	typedef std::map<CQtString, CQtComAutoPtr<CQtDnsRecord> > CacheRecordsType;
	CacheRecordsType m_CacheRecords;
	typedef std::list<CQtComAutoPtr<CQtDnsRecord> > PendingRecordsType;
	PendingRecordsType m_PendingRecords;

	class CObserverAndListener : public IQtEvent
	{
	public:
		CObserverAndListener(CQtDnsManager *aDnsManager, 
							 IQtObserver *aObserver, 
							 AQtThread *aThreadListener, 
							 int aError, 
							 const CQtString &aHostName)
			: m_pDnsManager(aDnsManager)
			, m_pObserver(aObserver)
			, m_pThreadListener(aThreadListener)
			, m_nError(aError)
			, m_strHostName(aHostName)
		{
			QT_ASSERTE(m_pDnsManager);
			QT_ASSERTE(m_pObserver);
			QT_ASSERTE(m_pThreadListener);
		}

		bool operator == (const CObserverAndListener &aRight)
		{
			return m_pObserver == aRight.m_pObserver;
		}

		bool operator == (IQtObserver *aObserver)
		{
			return m_pObserver == aObserver;
		}

		// interface IQtEvent
		virtual QtResult OnEventFire();

		CQtDnsManager *m_pDnsManager;
		IQtObserver *m_pObserver;
		AQtThread *m_pThreadListener;
		int m_nError;
		CQtString m_strHostName;
	};
	typedef std::vector<CObserverAndListener> ObserversType;
	ObserversType m_Observers;
	
	typedef CQtMutexThreadRecursive MutexType;
	MutexType m_Mutex;

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	static LRESULT CALLBACK DnsEventWndProc(
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND m_hwndDsnWindow;
	BOOL m_bIsEL98;			//the flag to id win98 or lower
#endif
	AQtThread *m_pThreadDNS;
//#endif // QT_WIN32_ENABLE_AsyncGetHostByName
	AQtThread *m_pThreadNetwork;
	CQtTimerWrapperID m_TimerExpired;

#if !defined QT_WIN32 && !defined QT_MACOS //for bug 372965
	MutexType m_ShutdownMutex;
#endif
};

#ifdef QT_WIN32
  #define QT_NETDB_BUF_SIZE MAXGETHOSTSTRUCT 
#else
  #define QT_NETDB_BUF_SIZE 1024 
#endif // QT_WIN32

class QT_OS_EXPORT CQtDnsRecord : public CQtReferenceControlMutilThread 
{
public:
	CQtDnsRecord(const CQtString &aHostName
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
		, BOOL bIsWin98
#endif
	);
	virtual ~CQtDnsRecord();

	// only support IPV4 now.
	class iterator
	{
	public:
		typedef DWORD value_type;

		iterator() : m_ppszAddr(NULL)
			{ }

		iterator(char *aBuf)
		{
			struct hostent *pHeResult = reinterpret_cast<hostent *>(aBuf);
			if  (pHeResult) {
				QT_ASSERTE(pHeResult->h_length == 4);
				m_ppszAddr = pHeResult->h_addr_list;
			}
			else
				m_ppszAddr = NULL;
		}

		iterator& operator++()
		{
			QT_ASSERTE(m_ppszAddr);
			if (m_ppszAddr) {
				m_ppszAddr++;
				if (*m_ppszAddr == NULL)
					m_ppszAddr = NULL;
			}
			return (*this); 
		}

		iterator operator++(int)
		{
			iterator tmp = *this;
			++*this;
			return tmp; 
		}

		value_type operator*() const
		{
			if (m_ppszAddr && *m_ppszAddr)
				return *(reinterpret_cast<value_type*>(*m_ppszAddr));
			else
				return INADDR_NONE;
		}

		bool operator==(const iterator &aRight) const
		{
			return (m_ppszAddr == aRight.m_ppszAddr); 
		}

		bool operator!=(const iterator &aRight) const
		{
			return (!(*this == aRight)); 
		}

	private:
		char ** m_ppszAddr;
	};
	iterator begin()
	{
		QT_ASSERTE(m_State == RSV_SUCCESS);
		if (m_State == RSV_SUCCESS)
			return iterator(m_szBuffer);
		else
			return iterator(NULL);
	}

	iterator end()
	{
		QT_ASSERTE(m_State == RSV_SUCCESS);
		return iterator(NULL);
	}

	CQtString GetHostName()
	{
		return m_strHostName;
	}

private:
	CQtString m_strHostName;
	enum {
		RSV_IDLE,
		RSV_PROCESSING,
		RSV_SUCCESS,
		RSV_FAILED,
	} m_State;
	CQtTimeValue m_tvResolve;
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	HANDLE m_HandleResolve;
	BOOL m_bIsEL98;
#else
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
	// contains struct hostent.
	char m_szBuffer[QT_NETDB_BUF_SIZE];

	friend class CQtDnsManager;
};

class QT_OS_EXPORT IDnsObserver : public IQtObserver
{
public:
	virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL) = 0;
	
	virtual ~IDnsObserver() {
		QT_INFO_TRACE_THIS("IDnsObserver::~IDnsObserver cancel the observe");
		CQtDnsManager *pDnsManager = CQtDnsManager::Instance();
		pDnsManager->CancelResolve(this);
	}
};


#endif // !QTDNSMANAGER_H
