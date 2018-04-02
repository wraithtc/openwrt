/*------------------------------------------------------*/
/* File channel to do FILE request                      */
/*                                                      */
/* QtChannelFile.h                                      */
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

#ifndef QTCHANNELFILE_H
#define QTCHANNELFILE_H

#include "QtHttpInterface.h"
#include "QtTimerWrapperID.h"

class CQtHttpUrl;

class QT_OS_EXPORT CQtChannelFile 
	: public IQtChannel 
	, public CQtReferenceControlSingleThreadTimerDelete
	, public CQtTimerWrapperIDSink
{
public:
	CQtChannelFile();
	virtual ~CQtChannelFile();

	BOOL Init(CQtHttpUrl *aUrl);
	BOOL Init(const CQtString &aPath);

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();

	// interface IQtChannel
	virtual QtResult AsyncOpen(IQtChannelSink *aSink);
	virtual QtResult GetUrl(CQtHttpUrl *&aUrl);

	// interface IQtTransport
	virtual QtResult OpenWithSink(IQtTransportSink *aSink);
	virtual IQtTransportSink* GetSink();
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL);
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult Disconnect(QtResult aReason);

	// interface CQtTimerWrapperIDSink
	virtual void OnTimer(CQtTimerWrapperID* aId);

private:
	CQtString m_strFileName;
	FILE *m_pfGet;
	IQtChannelSink *m_pSink;
	CQtTimerWrapperID m_TimerReadFile;
	BOOL m_bSync;
};

#endif // !QTCHANNELFILE_H
