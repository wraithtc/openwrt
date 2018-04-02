
#include "QtBase.h"
#include "QtInetAddr.h"
#include "QtDnsManager.h"

CQtInetAddr CQtInetAddr::s_InetAddrAny;

QtResult CQtInetAddr::Set(LPCSTR aHostName, WORD aPort)
{
	::memset(&m_SockAddr, 0, sizeof(m_SockAddr));
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_port = htons(aPort);

	QtResult rv = SetIpAddrByString(aHostName);
	if (QT_FAILED(rv)) {
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		m_strHostName = aHostName;
		m_bIsResolved = FALSE;
		rv = TryResolve();
#else
		CQtComAutoPtr<CQtDnsRecord> pRecord;
		QtResult rv = CQtDnsManager::Instance()->SyncResolve(
			pRecord.ParaOut(),
			aHostName);
		if (QT_SUCCEEDED(rv)) 
			SetIpAddrBy4Bytes(*pRecord->begin());
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	}
	return rv;
}

QtResult CQtInetAddr::Set(LPCSTR aIpAddrAndPort)
{
	WORD wPort = 0;
	QT_ASSERTE_RETURN(aIpAddrAndPort, QT_ERROR_INVALID_ARG);
	char *szFind = const_cast<char*>(::strchr(aIpAddrAndPort, ':'));
	if (!szFind) {
		QT_WARNING_TRACE_THIS("CQtInetAddr::Set, unknow aIpAddrAndPort=" << aIpAddrAndPort);
//		QT_ASSERTE(FALSE);
//		return QT_ERROR_INVALID_ARG;
		szFind = const_cast<char*>(aIpAddrAndPort) + strlen(aIpAddrAndPort);
		wPort = 0;
	}
	else {
		wPort = static_cast<WORD>(::atoi(szFind + 1));
	}

	// 256 bytes is enough, otherwise the ip string is possiblly wrong.
	char szBuf[256];
	int nAddrLen = szFind - aIpAddrAndPort;
	QT_ASSERTE_RETURN((size_t)nAddrLen < sizeof(szBuf), QT_ERROR_NOT_AVAILABLE);
	::memcpy(szBuf, aIpAddrAndPort, nAddrLen);
	szBuf[nAddrLen] = '\0';

	return Set(szBuf, wPort);
}

QtResult CQtInetAddr::SetIpAddrByString(LPCSTR aIpAddr)
{
	DWORD dwIp = INADDR_ANY;
	BOOL bAddrOk = IpAddrStringTo4Bytes(aIpAddr, dwIp);
	if (!bAddrOk) {
//		QT_ERROR_TRACE_THIS("CQtInetAddr::SetIpAddrByString, wrong aIpAddr=" << aIpAddr);
		return QT_ERROR_FAILURE;
	}

	return SetIpAddrBy4Bytes(dwIp);
}

CQtString CQtInetAddr::IpAddr4BytesToString(DWORD aIpDword)
{
#ifdef QT_WIN32
	struct in_addr inAddr;
	inAddr.s_addr = aIpDword;
	LPCSTR pAddr = ::inet_ntoa(inAddr);
#elif defined (QT_LINUX) || defined (QT_MACOS)
	char szBuf[INET_ADDRSTRLEN];
	LPCSTR pAddr = ::inet_ntop(AF_INET, &aIpDword, szBuf, sizeof(szBuf));
#else//by now, is QT_SOLARIS
	char szBuf[128];
	memset(szBuf, 0, 128);
	aIpDword = ntohl(aIpDword);
	sprintf(szBuf,"%d.%d.%d.%d", (aIpDword>>24), (aIpDword&0x00FFFFFF)>>16, (aIpDword&0x0000ffff)>>8, (aIpDword&0x000000ff));
	LPCSTR pAddr = szBuf;
#endif // QT_WIN32
	return CQtString(pAddr);
}

#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
QtResult CQtInetAddr::TryResolve()
{
	QT_ASSERTE_RETURN(!IsResolved(), QT_OK);

	// try to get ip addr from DNS
	CQtComAutoPtr<CQtDnsRecord> pRecord;
	QtResult rv = CQtDnsManager::Instance()->AsyncResolve(
		pRecord.ParaOut(),
		m_strHostName, 
		NULL);
	if (QT_SUCCEEDED(rv)) {
		DWORD dwIp = *(pRecord->begin());
		SetIpAddrBy4Bytes(dwIp);
	}
	else {
		QT_ASSERTE(!IsResolved());
	}
	return rv;
}
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

#if 0
DWORD CQtInetAddr::ResolveSynchAndCache(LPCSTR aHostName)
{
	// TODO: Async resolve.
	typedef std::map<CQtString, DWORD> Host2IpType;
	static Host2IpType s_mapHost2Ip;
	Host2IpType::iterator iter = s_mapHost2Ip.find(aHostName);
	if (iter != s_mapHost2Ip.end()) {
		return (*iter).second;
	}

	DWORD dwRet = INADDR_NONE;
	struct hostent *pHeResult = NULL;
#ifdef QT_LINUX
	struct hostent heResultBuf;
	struct hostent *pheResultBuf = &heResultBuf;
	char szHostentData[4*1024];
	int nHErr;
	if (::gethostbyname_r(aHostName, 
		pheResultBuf, 
		szHostentData, 
		sizeof(szHostentData), 
		&pheResultBuf,
		&nHErr) == 0)
	{
		pHeResult = &heResultBuf;
	}
#else // !QT_LINUX
	pHeResult = ::gethostbyname(aHostName);
#endif
	if (!pHeResult) {
		QT_ERROR_TRACE_THIS("CQtInetAddr::ResolveSynchAndCache, gethostbyname() failed!"
			" name=" << aHostName <<
			" err=" << errno);
		dwRet = INADDR_NONE;
	}
	else {
		::memcpy(&dwRet, pHeResult->h_addr, pHeResult->h_length);
		s_mapHost2Ip.insert(std::make_pair(CQtString(aHostName), dwRet));
	}
	return dwRet;
}
#endif
