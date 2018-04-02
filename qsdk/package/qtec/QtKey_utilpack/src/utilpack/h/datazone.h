/*------------------------------------------------------*/
/* data zone interface                                  */
/*                                                      */
/* datazone.h                                           */
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
#ifndef __QTEC_DATA_ZONE__
#define __QTEC_DATA_ZONE__

#include <vector>
#include "QtReferenceControl.h"

using namespace std;

class IDataZoneSink;
class CQtInetAddr;

class IDataZone//it should be singlton
{
public:
	static IDataZone* Instance();
	static void DestroyInstance();
public:
	virtual QtResult Init(const char* strBindIP,
					WORD nPort,
					CQtInetAddr* arNodeAddresses,
					int iNodeCount,
					DWORD dwZoneID,//
					BOOL bAlone,
					BOOL bReliable,
					IDataZoneSink * pSink) = 0;

	virtual void Destroy()  = 0;

	virtual QtResult SetNoLocalNodes(
					CQtInetAddr* arNodeAddresses,
					int iNodeCount) = 0;

	virtual QtResult SyncRealtimeData(const LPBYTE data,
									DWORD dwLen,
									DWORD dwRecvID = 0) = 0;

	virtual QtResult SyncBackupData(
								const LPBYTE data,
								DWORD dwLen,
								DWORD dwRecvID = 0,
								DWORD dwOwnerID = 0) = 0;

	virtual QtResult SelectNode(DWORD& dwNodeID, BOOL bLocal = TRUE) = 0;//select a node by round robin.

	virtual QtResult GetSelfNodeID(DWORD& dwSelfID) = 0;//get myselft id;
	virtual QtResult CheckNodeID(CQtInetAddr& arCheckedAddr, DWORD& dwNodeID) = 0;
	virtual QtResult IsLocalNode(DWORD dwNodeID, BOOL& bLocal) = 0;
};

class IDataZoneSink
{
public:
	virtual void OnNodeJoin(DWORD dwNodeID) = 0 ;
	virtual void OnNodeLeave(DWORD dwNodeID) = 0 ;
	virtual void OnDataRecv(DWORD dwOwnID, LPBYTE data, DWORD dwLength) = 0;
	virtual void OnDataRequest(DWORD dwOwnID, vector<DWORD>& recvArray) = 0;	
};

/*extern "C"
{
	IDataZone* CreateDataZone();
}*/

#endif//!__QTEC_DATA_ZONE__

