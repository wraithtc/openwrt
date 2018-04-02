/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Webex Application Module For Macintosh Platform 	       	               */
/*                                                                         */
/* QtThreadMacEventPatch.cp                                                    */
/*                                                                         */
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

#include "QtBase.h"
#include "QtThreadMacEventPatch.h"
#include "QtTimerQueueOrderedList.h"

#if 0
const CFStringRef	kCFQTEQTMPObserveName				= CFSTR("QTEQTMP_Observe");
const CFStringRef	kCFQTEQTMPKeyName					= CFSTR("QTEQTMP_KeyName");
const int			kObserverID							= 100;
#endif
CFStringRef		g_mmpKeyName = NULL;

pascal void ThreadMacEventPatchTimerProc(CFRunLoopTimerRef inTimer, void *inUserData);
pascal void ThreadMacEventPatchTimerProc(CFRunLoopTimerRef inTimer, void *inUserData)
{
	CQtThreadMacEventPatch* pThreadMacEventPatch = (CQtThreadMacEventPatch*)inUserData;
	if(NULL == pThreadMacEventPatch)
		return;
	
	pThreadMacEventPatch->ProcessTimer();
	return;
}

#if 0
void myCallback(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) 
{
  	unsigned long parameter = 0;
	CFRange range ;		
 
	CFDataRef bufferRef = (CFDataRef)CFDictionaryGetValue(userInfo, g_mmpKeyName);
	range = CFRangeMake(0,4);
	CFDataGetBytes(bufferRef,range,(unsigned char*)(&parameter));

	CQtThreadMacEventPatch *pThreadMacEventPatch = (CQtThreadMacEventPatch *)parameter;
	pThreadMacEventPatch->ProcessEventQueue();
}
#endif
// ------------------------------------------------------------------------
// TpMacSocketMessageHandler
// ------------------------------------------------------------------------
// our deferred call back 
pascal OSStatus
TpMacSocketMessageHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus			status = noErr;
	EventParamType	sanityCheck = nil;
	int *				voidRef = nil;
	
	status = GetEventParameter(inEvent,  //EventRef inEvent
							kParamNameTpSocketReference,	//EventParamName inName
							kParamTypeTpSocketReference, 	//EventParamType inDesiredType
							&sanityCheck,					//EventParamType * outActualType /* can be NULL */
  							sizeof(voidRef),				//UInt32 inBufferSize
							NULL,						//UInt32 * outActualSize /* can be NULL */
							&voidRef);					//void * outData
	if (nil != status)
		return status;

	if (kParamTypeTpSocketReference != sanityCheck)
		return 0;		// !!! should return error?
	
	CQtThreadMacEventPatch *pThreadMacEventPatch = (CQtThreadMacEventPatch *)inUserData;
	pThreadMacEventPatch->ProcessEventQueue();
	
	return noErr;
}


CQtThreadMacEventPatch::CQtThreadMacEventPatch()
	: m_pTimerQueue(NULL)
	, m_bStopped(FALSE)
{
	m_nTimer = NULL;
	m_TimerFrequency = (kEventDurationMillisecond * ((EventTime)10.0));
	m_handlerUPP = NewEventHandlerUPP(TpMacSocketMessageHandler);
	m_TpEventHandlerRef = NULL;
#if 0	
	ProcessSerialNumber thePSN;
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	OSErr 	theErr;
	pid_t	thePID;
	theErr = GetProcessPID(&thePSN, &thePID);
	m_nObserverID = thePID;
	char cMMPObserveName[255];
	sprintf(cMMPObserveName,"QTEQTMP_Observe_%d",m_nObserverID);
	m_cfMMPObserveName = ::CFStringCreateWithCString(NULL,cMMPObserveName, CFStringGetSystemEncoding());
	char cMMPKeyName[255];
	sprintf(cMMPKeyName,"QTEQTMP_KeyName_%d",m_nObserverID);
	g_mmpKeyName = ::CFStringCreateWithCString(NULL,cMMPKeyName, CFStringGetSystemEncoding());
#endif	
}

CQtThreadMacEventPatch::~CQtThreadMacEventPatch()
{	
	QT_INFO_TRACE_THIS("CQtThreadMacEventPatch::~CQtThreadMacEventPatch()");
	if (m_nTimer)
	{
		CFRunLoopTimerInvalidate(m_nTimer);
		CFRelease(m_nTimer);
		m_nTimer = NULL;
	}
	if (m_TpEventHandlerRef)
	{
		RemoveEventHandler(m_TpEventHandlerRef);
		m_TpEventHandlerRef = NULL;
	}
	if (m_handlerUPP)
	{
		DisposeEventHandlerUPP(m_handlerUPP);
		m_handlerUPP = NULL;
	}
#if 0	
	CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
	CFNotificationCenterRemoveObserver(center, &m_nObserverID, m_cfMMPObserveName,NULL);
	if (m_cfMMPObserveName)
	{
		::CFRelease(m_cfMMPObserveName);
		m_cfMMPObserveName = NULL;
	}	
	if (g_mmpKeyName)
	{
		::CFRelease(g_mmpKeyName);
		g_mmpKeyName = NULL;
	}
#endif	
}

void CQtThreadMacEventPatch::OnThreadInit()
{
	Reset2CurrentThreadId();
	
	QT_ASSERTE(!m_pTimerQueue);
	m_pTimerQueue = new CQtTimerQueueOrderedList(NULL);
#if 0	
	EventTypeSpec		eventType;
	eventType.eventClass = kEventClassWebExTpEvent;
	eventType.eventKind = kEventKindWebExTpEvent;

	if (!m_TpEventHandlerRef)
	{
	InstallEventHandler(
		GetApplicationEventTarget(),		// EventTargetRef inTarget
			m_handlerUPP,						// EventHandlerUPP inHandler
		1,								// UInt32 inNumTypes
		&eventType,						// const EventTypeSpec * inList
		this,								// void *                 inUserData,
		  	&m_TpEventHandlerRef);							// EventHandlerRef * outRef	
	}
#endif	
	if(!m_nTimer)
	{
		CFRunLoopTimerContext context = { 0, this, NULL, NULL, NULL };
		m_nTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(),
										  m_TimerFrequency, 0, 0,
										  ThreadMacEventPatchTimerProc,
										  &context);
		if(m_nTimer)
		{
			CFRunLoopAddTimer(CFRunLoopGetCurrent(),
							  m_nTimer,
							  kCFRunLoopCommonModes);
		}
	}							
#if 0	
	CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();	    
	CFNotificationCenterAddObserver(center, &m_nObserverID, myCallback, 
							m_cfMMPObserveName,NULL, CFNotificationSuspensionBehaviorDeliverImmediately);	
#endif												
}

void CQtThreadMacEventPatch::OnThreadRun()
{
	RunApplicationEventLoop();
}

QtResult CQtThreadMacEventPatch::Stop(CQtTimeValue* aTimeout)
{
	QT_INFO_TRACE_THIS("CQtThreadMacEventPatch::Stop");
	if (m_nTimer)
	{
		CFRunLoopTimerInvalidate(m_nTimer);
		CFRelease(m_nTimer);
		m_nTimer = NULL;
	}
	// todo, let RunApplicationEventLoop() return.
	QuitApplicationEventLoop();
	
	return QT_OK;
}

IQtEventQueue* CQtThreadMacEventPatch::GetEventQueue()
{
	return this;
}

IQtTimerQueue* CQtThreadMacEventPatch::GetTimerQueue()
{
	return m_pTimerQueue;
}

QtResult CQtThreadMacEventPatch::PostEvent(IQtEvent *aEvent, EPriority aPri)
{
	QtResult rv = CQtEventQueueUsingMutex::PostEvent(aEvent, aPri);
	if (QT_FAILED(rv))
		return rv;
		
	// notify main thread.
	TpMacSocketMessage();
	
	return QT_OK;
}

QtResult CQtThreadMacEventPatch::ProcessEventQueue()
{
	DWORD dwRemainSize = 0;
	CQtEventQueueBase::EventsType listEvents;
	QtResult rv = CQtEventQueueUsingMutex::PopPendingEventsWithoutWait(
		listEvents, 1000/*CQtEventQueueBase::MAX_GET_ONCE*/, &dwRemainSize);
		
	
	if (QT_SUCCEEDED(rv))
		rv = CQtEventQueueUsingMutex::ProcessEvents(listEvents);
	if (dwRemainSize)
	{
	//	TpMacSocketMessage();
	//	DebugStr("\pCQtThreadMacEventPatch::ProcessEventQueue dwRemainSize>0");
		QT_INFO_TRACE_THIS("CQtThreadMacEventPatch::ProcessEventQueue dwRemainSize = "<<dwRemainSize );
	}	
	return rv;
}

QtResult CQtThreadMacEventPatch::ProcessTimer()
{
	if (m_pTimerQueue)
		m_pTimerQueue->CheckExpire();
	//	
	ProcessEventQueue();
	//
	return QT_OK;
}


// ------------------------------------------------------------------------
// TpMacSocketMessage
// ------------------------------------------------------------------------
//
void
CQtThreadMacEventPatch::TpMacSocketMessage()
{
#if 0
	EventRef theTpSocketEvent = NULL;

	try
	{
		OSStatus err;

		err = CreateEvent(NULL, 						//CFAllocatorRef inAllocator
						kEventClassWebExTpEvent,		//UInt32 inClassID
						kEventKindWebExTpEvent,	//UInt32 kind
						(EventTime)0.0,				//EventTime when
						kEventAttributeUserEvent,		//EventAttributes flags
						&theTpSocketEvent);	//EventRef* outEvent

	
		err = SetEventParameter(theTpSocketEvent, 	// EventRef inEvent
							kParamNameTpSocketReference,	// EventParamName   inName
							kParamTypeTpSocketReference,	// EventParamType inType
							sizeof(void*),				// UInt32 inSize
							this);						// const void * inDataPtr
		
		err = PostEventToQueue(GetMainEventQueue(),
							theTpSocketEvent,
	  						kEventPriorityLow);

	}
	catch(...)
	{
		if (NULL != theTpSocketEvent)
			ReleaseEvent(theTpSocketEvent);
	}
#endif	
#if 0
	CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
		
	unsigned long parameter = (unsigned long)this;

    CFMutableDictionaryRef stockInfoDict = CFDictionaryCreateMutable(NULL, 4,
									&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDataRef parameterRef = CFDataCreate(NULL,(UInt8*)&parameter,4);
	CFDictionaryAddValue(stockInfoDict, g_mmpKeyName, parameterRef);
	CFRelease(parameterRef);

	CFNotificationCenterPostNotification(center, m_cfMMPObserveName, NULL, stockInfoDict, FALSE);
    CFRelease(stockInfoDict);
#endif		
}


