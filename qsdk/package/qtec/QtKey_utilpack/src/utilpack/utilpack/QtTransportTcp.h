/*------------------------------------------------------*/
/* TCP transport                                        */
/*                                                      */
/* QtTransportTcp.h                                     */
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

#ifndef QTTRANSPORTTCP_H
#define QTTRANSPORTTCP_H

#include "QtTransportBase.h"
#include "QtSocket.h"

class QT_OS_EXPORT CQtTransportTcp : public CQtTransportBase 
{
public:
	CQtTransportTcp(IQtReactor *pReactor);
	virtual ~CQtTransportTcp();

	// interface AQtEventHandler
	virtual QT_HANDLE GetHandle() const ;
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);
	virtual int OnOutput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	// interface IQtTransport
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL);
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);

	CQtSocketTcp& GetPeer();

protected:
	virtual QtResult Open_t();
	virtual QtResult Close_t(QtResult aReason);

	int Recv_i(LPSTR aBuf, DWORD aLen);
	int Send_i(LPCSTR aBuf, DWORD aLen);

protected:
	CQtSocketTcp m_SocketTcp;
	BOOL m_bNeedOnSend;
	AQtThread *m_pThread;
};


// inline functions
inline int CQtTransportTcp::Recv_i(LPSTR aBuf, DWORD aLen)
{
	// the recv len must be as large as possible
	// due to avoid lossing real-time signal
	QT_ASSERTE(aBuf && aLen > 0);
	int nRecv = m_SocketTcp.Recv(aBuf, aLen);
	//Victor 12/21 2006
	if((nRecv < 0 &&  (EWOULDBLOCK == errno)) || //be interrupt
		nRecv == static_cast<int>(aLen)) //maybe not get all
	{
		int nAvailable = 0;
		if(m_SocketTcp.Control(FIONREAD, &nAvailable) != -1)
		{
			if(nAvailable > 0) //receive buff have others data
			{
/*
				QT_INFO_TRACE_THIS("CQtTransportTcp::Recv_i try read again, errno = " << errno << 
					" nAvailable = " << nAvailable);
*/
				CQtReceiveEvent *pEvent = new CQtReceiveEvent(this);
				IQtEventQueue *pQueue = m_pThread->GetEventQueue();
				if(pQueue)
					pQueue->PostEvent(pEvent);
				else
				{
					QT_ASSERTE(pQueue);
					delete pEvent;
				}
			}
		}
		else
		{
			QT_ERROR_TRACE_THIS("CQtTransportTcp::Recv_i can not access FIONREAD");
		}
		if(nRecv < 0 && (EWOULDBLOCK == errno))
		{
//			QT_WARNING_TRACE_THIS("CQtTransportTcp::Recv_i interrupt by signal, errno = " << errno);
			return -2;
		}
		else
		{
/*
			QT_WARNING_TRACE_THIS("CQtTransportTcp::Recv_i can not get all data in receive buff" << 
				" nAvailable = " << nAvailable);
*/
		}
	}
	if (nRecv < 0) {
		CQtErrnoGuard egTmp;
		QT_WARNING_TRACE_THIS("CQtTransportTcp::Recv_i, recv() failed!"
			" fd=" << m_SocketTcp.GetHandle() << 
			" err=" << errno);
		return -1;
	}
	if (nRecv == 0) {
//		QT_WARNING_TRACE_THIS("CQtTransportTcp::Recv_i, recv() 0!"
//			" fd=" << m_SocketTcp.GetHandle() << 
//			" err=" << errno);
//		QT_INFO_TRACE_THIS("CQtTransportTcp::Recv_i 0 graceful disconnect by remote host");
		// it is a graceful disconnect
		return -1;
	}
	return nRecv;
}

inline int CQtTransportTcp::Send_i(LPCSTR aBuf, DWORD aLen)
{
	QT_ASSERTE(aBuf && aLen > 0);
	int nSend = m_SocketTcp.Send(aBuf, aLen);
	
	if (nSend < 0) {
		if (errno == EWOULDBLOCK)
			return 0;
		else {
			CQtErrnoGuard egTmp;
			QT_WARNING_TRACE_THIS("CQtTransportTcp::Send_i, send() failed!"
				" fd=" << m_SocketTcp.GetHandle() << 
				" err=" << errno);
			return -1;
		}
	}
	return nSend;
}

#endif // !QTTRANSPORTTCP_H
