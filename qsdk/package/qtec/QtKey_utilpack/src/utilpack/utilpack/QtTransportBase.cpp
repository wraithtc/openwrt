
#include "QtBase.h"
#include "QtTransportBase.h"
#include "QtSocket.h"
#include "QtTimeValue.h"

#ifdef QT_WIN32
  #include <ws2tcpip.h>
#endif // QT_WIN32

#if defined (QT_SUPPORT_QOS) && defined (QT_WIN32)
  #include <Qos.h>
#endif // QT_SUPPORT_QOS && QT_WIN32

CQtTransportBase::CQtTransportBase(IQtReactor *pReactor)
	: m_pSink(NULL)
	, m_pReactor(pReactor)
{
	QT_ASSERTE(m_pReactor);
}

CQtTransportBase::~CQtTransportBase()
{
}

DWORD CQtTransportBase::AddReference()
{
	return CQtReferenceControlSingleThreadTimerDelete::AddReference();
}

DWORD CQtTransportBase::ReleaseReference()
{
	return CQtReferenceControlSingleThreadTimerDelete::ReleaseReference();
}

QtResult CQtTransportBase::OpenWithSink(IQtTransportSink *aSink)
{
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
//	QT_INFO_TRACE_THIS("CQtTransportBase::OpenWithSink pSink = " << aSink);

	// we allow the upper layer invokes this function many times.
	if (m_pSink) {
		m_pSink = aSink;
		return QT_OK;
	}
	else {
		m_pSink = aSink;
	}

	QtResult rv = Open_t();
	if (QT_FAILED(rv)) {
		Close_t(QT_OK);
		m_pSink = NULL;
	}
	return rv;
}

IQtTransportSink* CQtTransportBase::GetSink()
{
	return m_pSink;
}

QtResult CQtTransportBase::Disconnect(QtResult aReason)
{
//	QT_INFO_TRACE_THIS("CQtTransportBase::Disconnect areason = " << aReason << " pSink = " << m_pSink);
	QtResult rv = Close_t(aReason);
	m_pSink = NULL;
	return rv;
}

int CQtTransportBase::OnClose(QT_HANDLE aFd, MASK aMask)
{
	Close_t(QT_OK);
	IQtTransportSink *pTmp = m_pSink;
	m_pSink = NULL;

	QT_ASSERTE(pTmp);
	if (pTmp)
		pTmp->OnDisconnect(QT_ERROR_NETWORK_SOCKET_ERROR, this);
	return 0;
}

QtResult CQtTransportBase::SetTos2Socket(CQtSocketBase &aSocket, LPVOID aArg)
{
#ifdef QT_WIN32
	static BOOL s_bSetWin32Registry = FALSE;
	if (!s_bSetWin32Registry) {
		s_bSetWin32Registry = TRUE;
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!::GetVersionEx(&osvi)) {
			QT_ERROR_TRACE_THIS("CQtTransportBase::SetTos2Socket,"
				" GetVersionEx() failed! err=" << ::GetLastError());
		}

		QT_INFO_TRACE_THIS("CQtTransportBase::SetTos2Socket, PlatformId = " << osvi.dwPlatformId << " Major Version = " << osvi.dwMajorVersion);
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5) 
		{
			HKEY hKey;
			LONG lRet = ::RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, 
				"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
				0, 
				KEY_SET_VALUE, 
				&hKey);
			if (lRet == ERROR_SUCCESS) {
				DWORD dwValue = 0;
				lRet = ::RegSetValueEx(
					hKey, 
					"DisableUserTOSSetting", 
					0, 
					REG_DWORD, 
					(LPBYTE)&dwValue, 
					sizeof(DWORD));
				if (lRet != ERROR_SUCCESS) {
					QT_ERROR_TRACE_THIS("CQtTransportBase::SetTos2Socket,"
						" RegSetValueEx() failed! lRet=" << lRet);
				}
			}
			else {
				QT_ERROR_TRACE_THIS("CQtTransportBase::SetTos2Socket,"
					" RegOpenKeyEx() failed! lRet=" << lRet);
			}
		}
	}
#endif // QT_WIN32

	int nTos = 1;
	int nLen = sizeof(int);
	if (aSocket.GetOption(IPPROTO_IP, IP_TOS, &nTos, &nLen) == -1) {
		QT_ERROR_TRACE_THIS("CQtTransportBase::SetTos2Socket, GetOption(IP_TOS) failed! err=" << errno);
	}
	if (aSocket.SetOption(IPPROTO_IP, IP_TOS, aArg, sizeof(int)) == -1) {
		QT_ERROR_TRACE_THIS("CQtTransportBase::SetTos2Socket, SetOption(IP_TOS) failed! err=" << errno);
		return QT_ERROR_NETWORK_SOCKET_ERROR;
	}
	else
		return QT_OK;
}

#ifdef QT_SUPPORT_QOS
QtResult CQtTransportBase::SetQos2Socket(QT_HANDLE aSocket)
{
	QT_ASSERTE(aSocket != QT_INVALID_HANDLE);

#ifdef QT_WIN32
	QOS Qos;
	::memset(&Qos, QOS_NOT_SPECIFIED, sizeof(QOS));
	
	Qos.ReceivingFlowspec.ServiceType           = SERVICETYPE_NOTRAFFIC;
    Qos.ReceivingFlowspec.TokenRate             = QOS_NOT_SPECIFIED; 
    Qos.ReceivingFlowspec.TokenBucketSize       = QOS_NOT_SPECIFIED; 
    Qos.ReceivingFlowspec.PeakBandwidth         = QOS_NOT_SPECIFIED; 
	Qos.ReceivingFlowspec.Latency			    = QOS_NOT_SPECIFIED;
	Qos.ReceivingFlowspec.DelayVariation        = QOS_NOT_SPECIFIED; 
    Qos.ReceivingFlowspec.MinimumPolicedSize    = QOS_NOT_SPECIFIED; 
    Qos.ReceivingFlowspec.MaxSduSize            = QOS_NOT_SPECIFIED; 
    
	Qos.ProviderSpecific.len = 0;
    Qos.ProviderSpecific.buf = NULL;
	
#ifndef SERVICETYPE_QUALITATIVE
  #define SERVICETYPE_QUALITATIVE 0x0000000D
#endif // SERVICETYPE_QUALITATIVE

    Qos.SendingFlowspec.ServiceType             = SERVICETYPE_QUALITATIVE; 
    Qos.SendingFlowspec.TokenRate               = QOS_NOT_SPECIFIED; 
    Qos.SendingFlowspec.TokenBucketSize         = QOS_NOT_SPECIFIED; 
    Qos.SendingFlowspec.PeakBandwidth           = QOS_NOT_SPECIFIED; 
	Qos.SendingFlowspec.Latency			        = QOS_NOT_SPECIFIED;
	Qos.SendingFlowspec.DelayVariation          = QOS_NOT_SPECIFIED; 
    Qos.SendingFlowspec.MinimumPolicedSize      = QOS_NOT_SPECIFIED; 
    Qos.SendingFlowspec.MaxSduSize              = QOS_NOT_SPECIFIED;
/*
	//g711
	Qos.SendingFlowspec.ServiceType             = SERVICETYPE_GUARANTEED; 
    Qos.SendingFlowspec.TokenRate               = 9250; 
    Qos.SendingFlowspec.TokenBucketSize         = 680; 
    Qos.SendingFlowspec.PeakBandwidth           = 13875; 
	Qos.SendingFlowspec.Latency			        = QOS_NOT_SPECIFIED;
	Qos.SendingFlowspec.DelayVariation          = QOS_NOT_SPECIFIED; 
    Qos.SendingFlowspec.MinimumPolicedSize      = 340; 
    Qos.SendingFlowspec.MaxSduSize              = 340; 
	
	QOS_PRIORITY qosPriority;
	::memset(&qosPriority, 0, sizeof(qosPriority));
	qosPriority.ObjectHdr.ObjectType   = QOS_OBJECT_PRIORITY;
	qosPriority.ObjectHdr.ObjectLength = sizeof(qosPriority);
	qosPriority.SendPriority = 184;

	Qos.ProviderSpecific.len = sizeof(qosPriority);
	Qos.ProviderSpecific.buf = reinterpret_cast<char*>(&qosPriority);
*/
	QOS_TRAFFIC_CLASS qosTraffic;
	::memset(&qosTraffic, 0, sizeof(qosTraffic));
	qosTraffic.ObjectHdr.ObjectType   = QOS_OBJECT_TRAFFIC_CLASS;
	qosTraffic.ObjectHdr.ObjectLength = sizeof(qosTraffic);
	qosTraffic.TrafficClass = 1;

	Qos.ProviderSpecific.len = sizeof(qosTraffic);
	Qos.ProviderSpecific.buf = reinterpret_cast<char*>(&qosTraffic);

	DWORD dwBytesRet;
	int nRet = ::WSAIoctl((QT_SOCKET)aSocket, SIO_SET_QOS, &Qos, sizeof(QOS), NULL, 0, &dwBytesRet, NULL, NULL);
	if (nRet == SOCKET_ERROR) {
		QT_ERROR_TRACE_THIS("CQtTransportBase::SetQos2Socket, WSAIoctl() failed!"
			" err=" << WSAGetLastError());
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;

#else
	return QT_ERROR_NOT_IMPLEMENTED;
#endif // QT_WIN32
}

#endif // QT_SUPPORT_QOS

//////////////////////////////////////////////////////////////////////////
QtResult CQtReceiveEvent::OnEventFire()
{
	QT_ASSERTE_RETURN(m_pTransport, QT_ERROR_FAILURE);
	m_pTransport->OnInput();
	return QT_OK;
}