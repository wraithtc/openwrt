/*------------------------------------------------------*/
/* Thread manager                                       */
/*                                                      */
/* QtThreadManager.h                                    */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*	2017/02/15   Create                             */
/*                                                      */
/*------------------------------------------------------*/

#ifndef QTTHREADMANAGER_H
#define QTTHREADMANAGER_H

#include "QtDefines.h"
#include "QtMutex.h"
#include <vector>

class CQtTimeValue;
class AQtThread;
class IQtReactor;
class IQtEventQueue;
class IQtTimerQueue;

/**
 *  CQtThreadManager must be declared in the stack of main fuction!
 *  For example for the usage:
 *    int main(int argc, char** argv)
 *    {
 *      CQtThreadManager theThreadManager;
 *      theThreadManager.InitMainThread(argc, argv);
 *      theThreadManager.GetThread(CQtThreadManager::MAIN)->OnThreadRun();
 *    }
 */
class QT_OS_EXPORT CQtThreadManager  
{
public:
	CQtThreadManager();
	~CQtThreadManager();
	
	static CQtThreadManager* Instance();
	static void CleanupOnlyOne();

	QtResult InitMainThread(int aArgc, char** aArgv);
	
	typedef int TType;
	enum 
	{
		TT_MAIN,
		TT_NETWORK,
		TT_DNS,
		TT_CURRENT,
		TT_TIMER,
		TT_UNKNOWN = -1,
		
		// This private thread type is used by applications. 
		TT_USER_DEFINE_BASE = 1000,
		TT_USER_SESSION = TT_USER_DEFINE_BASE + 1000,
		TT_CLIENT_SESSION = TT_USER_SESSION,
		TT_AB_SESSION,
		TT_SESSION_QTD
	};

	enum TFlag
	{
		TF_NONE = 0,
		TF_JOINABLE = (1 << 0),
		TF_DETACHED = (1 << 1)
	};

	AQtThread* GetThread(TType aType);
	IQtReactor* GetThreadReactor(TType aType);
	IQtEventQueue* GetThreadEventQueue(TType aType);
	IQtTimerQueue* GetThreadTimerQueue(TType aType);

	// create Reactor Thread (mainly for network)
//	QtResult CreateUserReactorThread(AQtThread *&aThread);

	// create Task Thread (include EventQueue and TimerQueue only)
	///Add a TType parameters to let user can specify the thread type, 
	//if use default value, the type will set by system automatic
	QtResult CreateUserTaskThread(
		AQtThread *&aThread, 
		TFlag aFlag = TF_JOINABLE,
		BOOL bWithTimerQueue = TRUE, TType aType = TT_UNKNOWN);

	// get user thread type (TT_USER_DEFINE_BASE + x)
	TType GetAndIncreamUserType();

	static void SleepMs(DWORD aMsec);

public:
	// the following member functions are mainly used in TP.
	QtResult CreateReactorThread(TType aType, IQtReactor *aReactor, AQtThread *&aThread);
	
	// mainly invoked by CQtThreadManager::~CQtThreadManager()
	QtResult StopAllThreads(CQtTimeValue* aTimeout = NULL);
	QtResult JoinAllThreads();

	// mainly invoked by AQtThread::Create().
	QtResult RegisterThread(AQtThread* aThread);

	QtResult UnregisterThread(AQtThread* aThread);

	void GetSingletonMutex(CQtMutexThreadRecursive *&aMutex)
	{
		aMutex = &m_SingletonMutex;
	}

	void GetReferenceControlMutex(CQtMutexThread *&aMutex)
	{
		aMutex = &m_ReferenceControlMutexThread;
	}

	// thread module
	enum TModule
	{
		TM_SINGLE_MAIN,
		TM_MULTI_ONE_DEDICATED,
		TM_MULTI_POOL
	};
	static TModule GetNetworkThreadModule();
	
	static QT_THREAD_ID GetThreadSelfId()
	{
#ifdef WIN32
		return ::GetCurrentThreadId();
#else
		return ::pthread_self();
#endif // WIN32
	}
	
	static BOOL IsThreadEqual(QT_THREAD_ID aT1, QT_THREAD_ID aT2)
	{
#ifdef WIN32
		return aT1 == aT2;
#else
	#if defined (MACOS) && !defined(MachOSupport)
		return CFM_pthread_equal(aT1, aT2);
	#else
		return ::pthread_equal(aT1, aT2);
	#endif	
#endif // WIN32
	}

	static BOOL IsEqualCurrentThread(QT_THREAD_ID aId)
	{
		return IsThreadEqual(aId, GetThreadSelfId());
	}

	static BOOL IsEqualCurrentThread(TType aType);

	// mainly for init winsock2
	static QtResult SocketStartup();
	static QtResult SocketCleanup();

	static IQtReactor* CreateNetworkReactor();

private:
#ifdef WIN32
	static BOOL s_bSocketInited;
#endif // WIN32
	
	// singleton mutex is recursive due to CQtCleanUpBase() will lock it too.
	CQtMutexThreadRecursive m_SingletonMutex;
	CQtMutexThread m_ReferenceControlMutexThread;

	typedef std::vector<AQtThread *> ThreadsType;
	ThreadsType m_Threads;
	typedef CQtMutexThread MutexType ;
	MutexType m_Mutex;
	TType m_TTpyeUserThread;

private:
	// = Prevent assignment and initialization.
	void operator = (const CQtThreadManager&);
	CQtThreadManager(const CQtThreadManager&);
};

#endif // !QTTHREADMANAGER_H
