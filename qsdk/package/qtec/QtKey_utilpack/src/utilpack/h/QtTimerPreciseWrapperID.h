#ifndef QTTIMERPRECISEWRAPPERID_H
#define QTTIMERPRECISEWRAPPERID_H

#include "QtBase.h"
#include "QtReactorInterface.h"

class CQtTimerWrapperID;
class CQtTimerQueueCalendar;
class CQtTimerPreciseWrapperID;


class QT_OS_EXPORT CQtTimerPreciseWrapperIDSink
{
public:
	virtual void OnTimer(CQtTimerPreciseWrapperID* aId) = 0;

protected:
	virtual ~CQtTimerPreciseWrapperIDSink(){};
};

/********************************************************
	For 10ms high resolution timer.
 ********************************************************/
class QT_OS_EXPORT CQtTimerPreciseManager
: public AQtThread
, public CQtStopFlag
{
private:
	CQtTimerPreciseManager();
	CQtTimerPreciseManager(const CQtTimerPreciseManager &);
	CQtTimerPreciseManager & operator=(const CQtTimerPreciseManager &);
public:
	virtual ~CQtTimerPreciseManager();
	
	static CQtTimerPreciseManager* Instance();

	//Implement interface AQtThread.
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	virtual QtResult Stop(CQtTimeValue* aTimeout = NULL);

	QtResult Schedule(
		CQtTimerPreciseWrapperID* aTimerId,
		CQtTimerPreciseWrapperIDSink* aSink,
		const CQtTimeValue &aInterval,
		DWORD aCount = 0);
	QtResult Cancel(CQtTimerPreciseWrapperID* aTimerId);
	
	
private:

	CQtTimerQueueCalendar* m_pTimerQueue;
	WORD m_wTimerRes; //timer resolution
	WORD m_wTimerId;  //timer ID

	CQtMutexThread m_Mutex;
};

/********************************************************
	CQtTimerPreciseWrapperID,10ms high resolution timer.
 ********************************************************/
class QT_OS_EXPORT CQtTimerPreciseWrapperID
: public IQtTimerHandler
{
public:
	CQtTimerPreciseWrapperID();
	virtual ~CQtTimerPreciseWrapperID();

	/// Schedule an timer that will expire after <aInterval> for <aCount> times. 
	/// if <aCount> is 0, schedule infinite times.
	QtResult Schedule( CQtTimerPreciseWrapperIDSink* aSink,
		const CQtTimeValue &aInterval,
		DWORD aCount = 0,
		CQtThreadManager::TType aTType = CQtThreadManager::TT_MAIN);

	QtResult Cancel();

	//Implement interface IQtTimerHandler
	virtual void OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg);

public:
	BOOL GetStatus(){ return m_bScheduled; }

private:
	BOOL m_bScheduled;
	CQtThreadManager::TType m_TType;
	AQtThread* m_pThread; //for OnTimeOut callback.
	IQtEventQueue* m_pEventQueue;
};

/********************************************************
	For CEventTimer, by now. for CQtTimerPreciseWrapperID.
 ********************************************************/
class CEventTimer : public IQtEvent
{
public:
	CEventTimer(CQtTimerPreciseWrapperIDSink *pSink,CQtTimerPreciseWrapperID* pTimerId);
	virtual ~CEventTimer();
	
	//Implement IQtEvent;
	virtual QtResult OnEventFire();
	
private:
	CQtTimerPreciseWrapperIDSink* m_pSink;
	CQtTimerPreciseWrapperID* m_pTimerId;
};


#endif // !QTTIMERPRECISEWRAPPERID_H
