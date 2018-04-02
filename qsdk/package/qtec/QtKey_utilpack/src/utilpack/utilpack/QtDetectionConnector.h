/*------------------------------------------------------*/
/* Detection Connectior classe                          */
/*                                                      */
/* QtDetectionConnector.h                               */
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


#ifndef QT_DETECT_CONNECTOR_H
#define QT_DETECT_CONNECTOR_H

#include "QtConnectionInterface.h"
#include "QtReferenceControl.h"
#include "QtInetAddr.h"
#include "QtTimeValue.h"
#include <list>

#define TOP_PRIORITY					1
#define CONNECTOR_STATUS_UNCONNECTED	0x0001
#define CONNECTOR_STATUS_CONNECTED		0x0002

///////////////////////////////////////////
//class CQtDetectionConnector
///////////////////////////////////////////
class QT_OS_EXPORT CQtDetectionConnector : 
	public IQtDetectionConnector, 
	public CQtReferenceControlSingleThreadTimerDelete
{
	class CConnectorItem;
	friend class CConnectorItem;
	typedef std::list<CQtComAutoPtr<CConnectorItem> >::iterator iter_type;
public:
	/// add connection one by one, and the prior has high priority.
	virtual QtResult AddConnection(
		CQtConnectionManager::CType Type, 
		const CQtInetAddr &aAddrPeer,
		CQtTimeValue *aTimeDelay);
	
	/// Start connecting at the same time in <aTimeout>,
	/// If low priority connection connected, wait high priority connection in <aTimeDelay>.
	virtual void StartDetectionConnect(
		IQtAcceptorConnectorSink *aSink,
		CQtTimeValue *aTimeout = NULL);
	
	virtual void AsycConnect(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrPeer, 
		CQtTimeValue *aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL);
	
	virtual void CancelConnect(QtResult aReason);

	virtual BOOL IsConnector();
protected:
	// Cancel all Connect except pExclude.
	virtual void CancelConnect(CConnectorItem* pExclude, QtResult aReason);
public:
	CQtDetectionConnector();
	virtual ~CQtDetectionConnector();
public:	
	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::AddReference();
	};
	
	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::ReleaseReference();
	};
private:
	
	//Inner class, items of m_conn_list
	class CConnectorItem : 
		public IQtAcceptorConnectorSink, 
		public CQtReferenceControlSingleThread,
		public CQtTimerWrapperIDSink
	{
		friend class CQtDetectionConnector;
	public:
		CConnectorItem(
			IQtConnector *pIQtConnector,
			IQtConnector *pBackIQtConnector,
			CQtConnectionManager::CType aType, 
			WORD wPriority, 
			CQtInetAddr aAddrPeer, 
			CQtDetectionConnector* pOuterClass,
			CQtTimeValue* aTimeDelay);

		~CConnectorItem();

		void AsycConnect(CQtTimeValue *aTimeout = NULL);
	public:
		void IsAllFailed(QtResult aReason);

		virtual void OnConnectIndication(
			QtResult aReason,
			IQtTransport *aTrpt,
			IQtAcceptorConnectorId *aRequestId);
		
		void OnTimer(CQtTimerWrapperID* aId);
	public:	
		virtual DWORD AddReference()
		{
			return CQtReferenceControlSingleThread::AddReference();
		};
		
		virtual DWORD ReleaseReference()
		{
			return CQtReferenceControlSingleThread::ReleaseReference();
		};
		
		WORD GetStatus()
		{
			return m_wStatus;
		}

		void SetStatus(WORD wStatus)
		{
			m_wStatus = wStatus;
		}

		void SetAddrPeer(const CQtInetAddr &aAddr)
		{
			m_aAddrPeer = aAddr;
		}

		void CheckAndBeginStartDelay();

		void CancelConnect(QtResult aReason);

	private:
		CQtComAutoPtr<IQtConnector> m_pConnector;
		CQtComAutoPtr<IQtConnector> m_pBackConnector;
		QtResult m_aReason;									//Reason from TP Layer
		CQtComAutoPtr<IQtTransport> m_pITransport;			//Transport from TP layer
		CQtConnectionManager::CType m_aType;
		WORD m_wPriority;
		CQtInetAddr m_aAddrPeer;
		CQtDetectionConnector* m_pOuterClass;
		
		CQtTimerWrapperID m_Timer;	

		WORD m_wStatus;
		
		CQtTimeValue m_TimeDelay;

		CQtTimerWrapperID m_StartDelayTimer;
		CQtTimeValue m_StartDelay;
		CQtTimeValue m_SavedTimeoutByStartDelay;
		CQtTimeValue m_ConnectTimer;
		BOOL m_bStartDelayConnecting;
	};

private:
	WORD m_wPriority;
	BOOL m_bGetHighestPriority;								//Get the highest priority connection
	std::list<CQtComAutoPtr<CConnectorItem> > m_conn_list;
	IQtAcceptorConnectorSink *m_aSink;						//Connector Sink from Upper layer

	CQtComAutoPtr<IQtConnector> m_pChampionConnector;		//Final champion connector
	DWORD m_dwConnFailedCnt;								//How many conneciton failed
	DWORD m_dwConnAddCnt;									//how many connection added
};

#endif	//QT_DETECT_CONNECTOR_H
