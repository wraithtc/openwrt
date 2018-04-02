
#include "QtBase.h"
#include "QtDnsManager.h"
#include "QtThreadManager.h"

//////////////////////////////////////////////////////////////////////
// class CQtDnsManager
//////////////////////////////////////////////////////////////////////

//add by Victor 2006.4.28, to resolve host name from DNS server directly
//enable the dns resolved on MacOS
#if !defined QT_WIN32
#include <resolv.h>
#if defined QT_MACOS
#include <arpa/nameser_compat.h>
#endif

int __dns_decodename( unsigned char * packet, unsigned int offset, unsigned char * dest,
					 unsigned int maxlen, unsigned char * behindpacket ) 
{
	unsigned char * tmp;
	unsigned char * max = dest + maxlen;
	unsigned char * after = packet + offset;
	int ok = 0;
	for ( tmp = after; maxlen > 0 &&  * tmp; ) 
	{
		if ( tmp >= behindpacket ) return - 1;
		if ( (  * tmp >> 6 ) == 3 ) 
		{
			/* goofy DNS decompression */
			unsigned int ofs = ( ( unsigned int )(  * tmp & 0x3f ) << 8 ) |  * ( tmp + 1 );
			if ( ofs >= ( unsigned int )offset ) return - 1;
			/* RFC1035: "pointer to a _prior_ occurrance" */
			if ( after < tmp + 2 ) after = tmp + 2;
			tmp = packet + ofs;
			ok = 0;
		}
		else 
		{
			unsigned int duh;
			if ( dest +  * tmp + 1 > max ) return - 1;
			if ( tmp +  * tmp + 1 >= behindpacket ) return - 1;
			for ( duh =  * tmp; duh > 0; --duh )
				*dest++ =  * ++tmp;
			* dest++ = '.';
			ok = 1;
			++tmp;
			if ( tmp > after ) 
			{
				after = tmp;
				if ( ! * tmp ) ++after;
			}
		}
	}
	if ( ok ) --dest;
	* dest = 0;
	return after - packet;
}
int __dns_gethostbyx_r( const char * name, struct hostent * result,
					   char * buf, size_t buflen,
					   struct hostent ** RESULT, int * h_errnop, int lookfor ) 
{
#if defined QT_PORT_CLIENT && !defined QT_MACOS
	int rt = res_init();
	if(0 != rt) //init failed
	{
		QT_ERROR_TRACE("__dns_gethostbyx_r, res_init failed, rt = " << rt <<", errno = " << errno);
		return -1;
	}
	else
	{
		QT_INFO_TRACE("__dns_gethostbyx_r, res_init successful, rt = " << rt <<", errno = " << errno);
	}
#endif
	int names, ips;
	char * cur;
	char * max;
	unsigned char inpkg[1500];
	unsigned char * tmp;
	int size;
#ifndef QT_SOLARIS
	if ( lookfor == 1 ) 
	{
#endif
		result->h_addrtype = AF_INET;
		result->h_length = 4;
#ifndef QT_SOLARIS
	}
	else 
	{
		result->h_addrtype = AF_INET6;
		result->h_length = 16;
	}
#endif
	result->h_aliases = ( char **  )( buf + 8 * sizeof( char *  ) );
	result->h_addr_list = ( char **  )buf;
	result->h_aliases[0] = 0;
	cur = buf + 16 * sizeof( char *  );
	max = buf + buflen;
	names = ips = 0;
	if ( ( size = res_query( name, C_IN, lookfor, inpkg, 512 ) ) < 0 ) 
	{
invalidpacket :
		*h_errnop = HOST_NOT_FOUND;
		return - 1;
	}
	
	{
		tmp = inpkg + 12;
		
		{
			char Name[257];
			unsigned short q = ( ( unsigned short )inpkg[4] << 8 ) + inpkg[5];
			while ( q > 0 ) 
			{
				if ( tmp > inpkg + size ) goto invalidpacket;
				while (  * tmp ) 
				{
					tmp +=  * tmp + 1;
					if ( tmp > inpkg + size ) goto invalidpacket;
				}
				tmp += 5;
				--q;
			}
			if ( tmp > inpkg + size ) goto invalidpacket;
			q = ( ( unsigned short )inpkg[6] << 8 ) + inpkg[7];
			if ( q < 1 ) goto nodata;
			while ( q > 0 ) 
			{
				int decofs = __dns_decodename( ( unsigned char *  )inpkg, ( size_t )( tmp - inpkg ), ( unsigned char *  )Name, 256, inpkg + size );
				if ( decofs < 0 ) break;
				tmp = inpkg + decofs;
				--q;
				if ( tmp[0] != 0 || tmp[1] != lookfor ||/* TYPE != A */
					tmp[2] != 0 || tmp[3] != 1 ) 
				{
					/* CLASS != IN */
					if ( tmp[1] == 5 ) 
					{
						/* CNAME */
						tmp += 10;
						decofs = __dns_decodename( ( unsigned char *  )inpkg, ( size_t )( tmp - inpkg ), ( unsigned char *  )Name, 256, inpkg + size );
						if ( decofs < 0 ) break;
						tmp = inpkg + decofs;
					}
					else
						break;
					continue;
				}
				tmp += 10;
				/* skip type, class, TTL and length */
				{
					int slen;
					if ( lookfor == 1 || lookfor == 28 ) /* A or AAAA*/ 
					{
						slen = strlen( Name );
						if ( cur + slen + 8 + ( lookfor == 28 ? 12 : 0 ) >= max ) 
						{
							* h_errnop = NO_RECOVERY;
							return - 1;
						}
					}
					else if ( lookfor == 12 ) /* PTR */ 
					{
						decofs = __dns_decodename( ( unsigned char *  )inpkg, ( size_t )( tmp - inpkg ), ( unsigned char *  )Name, 256, inpkg + size );
						if ( decofs < 0 ) break;
						tmp = inpkg + decofs;
						slen = strlen( Name );
					}
					else
						slen = strlen( Name );
					strcpy( cur, Name );
					if ( names == 0 )
						result->h_name = cur;
					else
						result->h_aliases[names - 1] = cur;
					result->h_aliases[names] = 0;
					if ( names < 8 ) ++names;
					/*cur+=slen+1;
					*/
					cur += ( ( slen + 1 ) % 4 == 0 ? slen + 1  : ( ( slen + 1 ) / 4 + 1 ) * 4 );
					result->h_addr_list[ips++] = cur;
					if ( lookfor == 1 ) /* A */ 
					{
#ifndef QT_SOLARIS
						*( int *  )cur =  * ( int *  )tmp;
#else
						memcpy( cur, tmp, sizeof( char *  ) );
#endif
						cur += 4;
						result->h_addr_list[ips] = 0;
					}
					else if ( lookfor == 28 ) /* AAAA */ 
					{
						
						{
							int k;
							for ( k = 0; k < 16; ++k ) cur[k] = tmp[k];
						}
						cur += 16;
						result->h_addr_list[ips] = 0;
					}
				}
			}
		}
	}
	if ( !names ) 
	{
nodata :
		*h_errnop = NO_DATA;
		return - 1;
	}
	* h_errnop = 0;
	* RESULT = result;
	return 0;
}
int gethostbyname_dns_r( const char * name, struct hostent * result,
						char * buf, size_t buflen,
						struct hostent ** RESULT, int * h_errnop ) 
{
	QT_ASSERTE_RETURN(name, 1);
	*h_errnop = 0;
	size_t L = strlen( name );
	L = L % 4 == 0 ? L : ( L / 4 + 1 ) * 4;
	result->h_name = buf;
	if ( buflen < L ) 
	{
		* h_errnop = ERANGE;
		return 1;
	}
	strcpy( buf, name );
	return __dns_gethostbyx_r( name, result, buf + L, buflen - L, RESULT, h_errnop, 1 );
}
#endif
#define WIN32_DNS_CLASS_NAME "QtWin32DNSNotification"
#define WM_WIN32_DNS_EVENT WM_USER+133

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
ATOM g_atomDnsRegisterClass;
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

CQtDnsManager::CQtDnsManager()
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	: m_hwndDsnWindow(NULL)
#ifdef QT_ENABLE_DNS_THREAD //if enable select model, should not use wsadnsresolve
	, m_bIsEL98(TRUE)
#else
	, m_bIsEL98(FALSE)
#endif //!QT_ENABLE_DNS_THREAD
	,
#else
	: 
#endif
	m_pThreadDNS(NULL)

//#endif // QT_WIN32_ENABLE_AsyncGetHostByName
{
	m_pThreadNetwork = CQtThreadManager::Instance()->GetThread(
		CQtThreadManager::TT_NETWORK);
	QT_ASSERTE(m_pThreadNetwork);

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	BOOL bOsVersionInfoEx;
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		QT_WARNING_TRACE_THIS("CQtDnsManager::CQtDnsManager() GetVersionEx failed, err = " << ::GetLastError());
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		{
			QT_WARNING_TRACE_THIS("CQtDnsManager::CQtDnsManager() GetVersionEx failed, err = " << ::GetLastError());
		}
	}

	if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
	{
		if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion <= 10)
		{
			QT_INFO_TRACE_THIS("CQtDnsManager::CQtDnsManager() is win98 or lower!");
			m_bIsEL98 = TRUE;
		}
	}
	// we must create window in network thread.
	if(!m_bIsEL98)
	{
	QtResult rv;
	if (m_pThreadNetwork && CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId())) {
		rv = CreateDnsWindow();
	}
	else {
		// we have to use SendEvent() to wait CreateDnsWindow() finished.
		rv = m_pThreadNetwork->GetEventQueue()->SendEvent(this);
	}
	QT_ASSERTE(QT_SUCCEEDED(rv));
	}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

	// schedule timer in the main thread that cooperating with ~CQtDnsManager().
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(CQtThreadManager::TT_MAIN));
	m_TimerExpired.Schedule(this, CQtTimeValue(3, 0));

#if defined QT_PORT_CLIENT && defined QT_MACOS
	int rt = res_init();
	if(0 != rt) //init failed
	{
		QT_ERROR_TRACE("CQtDnsManager::CQtDnsManager, res_init failed, rt = " << rt <<", errno = " << errno);
	}
	else
	{
		QT_INFO_TRACE("CQtDnsManager::CQtDnsManager, res_init successful, rt = " << rt <<", errno = " << errno);
	}
#endif

}

CQtDnsManager::~CQtDnsManager()
{
	QT_INFO_TRACE_THIS("CQtDnsManager::~CQtDnsManager()");
	Shutdown();
}

CQtDnsManager* CQtDnsManager::Instance()
{
	return CQtSingletonT<CQtDnsManager>::Instance();
}

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
QtResult CQtDnsManager::CreateDnsWindow()
{
	QT_ASSERTE(!m_bIsEL98);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	QT_ASSERTE_RETURN(!m_hwndDsnWindow, QT_ERROR_ALREADY_INITIALIZED);

	DWORD dwCount = GetTickCount();
	char szClassName[MAX_PATH];
	snprintf(szClassName,sizeof(szClassName),"%s_%d",WIN32_DNS_CLASS_NAME,dwCount);
	//QT_INFO_TRACE_THIS("CQtDnsManager::CreateDnsWindow,register class " << szClassName);
	if (g_atomDnsRegisterClass == 0) {
		WNDCLASS wc;
		wc.style = 0;
		wc.lpfnWndProc = DnsEventWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(void*);
		wc.hInstance = NULL;
		wc.hIcon = 0;
		wc.hCursor = 0;
		wc.hbrBackground = 0;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = (TCHAR *)(szClassName);
		if ((g_atomDnsRegisterClass = ::RegisterClass(&wc)) == 0) {
			QT_WARNING_TRACE_THIS("CQtDnsManager::CreateDnsWindow, RegisterClass() failed!"
					" err=" << ::GetLastError());
			goto fail;
		}
	}

	m_hwndDsnWindow = ::CreateWindow(szClassName, NULL, 
		WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL, NULL, 0);
	if (!m_hwndDsnWindow) {
		QT_WARNING_TRACE_THIS("CQtDnsManager::CreateDnsWindow, CreateWindow() failed!"
			" err=" << ::GetLastError());
		goto fail;
	}

	::SetLastError(0);
	if (::SetWindowLong(m_hwndDsnWindow, 0, (LONG)this) == 0  && 
		::GetLastError() != 0) 
	{
		QT_WARNING_TRACE_THIS("CQtDnsManager::CreateDnsWindow, SetWindowLong() failed!"
			" err=" << ::GetLastError());
		goto fail;
	}
	return QT_OK;
	
fail:
	if (m_hwndDsnWindow) {
		::DestroyWindow(m_hwndDsnWindow);
		m_hwndDsnWindow = NULL;
	}
	return QT_ERROR_UNEXPECTED;
}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

QtResult CQtDnsManager::
AsyncResolve(CQtDnsRecord *&aRecord, const CQtString &aHostName, 
			 IQtObserver *aObserver, BOOL aBypassCache, 
			 AQtThread *aThreadListener)
{
	QT_ASSERTE(!aRecord);
	QT_INFO_TRACE_THIS("CQtDnsManager::AsyncResolve,"
		" aHostName=" << aHostName << 
		" aObserver=" << aObserver << 
		" aBypassCache=" << aBypassCache << 
		" aThreadListener=" << aThreadListener);

	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	if (!aBypassCache) {
		QtResult rv = FindInCache_l(aRecord, aHostName);
		if (QT_SUCCEEDED(rv))
			return rv;
	}
	CQtComAutoPtr<CQtDnsRecord> pRecordNew = new CQtDnsRecord(aHostName
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
		, m_bIsEL98
#endif
	);
	int nErr = BeginResolve_l(pRecordNew.ParaIn());
	if (nErr) {
		Resolved_l(pRecordNew.ParaIn(), nErr, FALSE);
		return QT_ERROR_FAILURE;
	}

	TryAddObserver_l(aObserver, aThreadListener, aHostName);
	return QT_ERROR_WOULD_BLOCK;
}

QtResult CQtDnsManager::
SyncResolve(CQtDnsRecord *&aRecord, const CQtString &aHostName, BOOL aBypassCache)
{
	QT_ASSERTE(!aRecord);
	QT_INFO_TRACE_THIS("CQtDnsManager::SyncResolve,"
		" aHostName=" << aHostName << 
		" aBypassCache=" << aBypassCache);

	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	if (!aBypassCache) {
		QtResult rv = FindInCache_l(aRecord, aHostName);
		if (QT_SUCCEEDED(rv))
			return rv;
	}

	CQtComAutoPtr<CQtDnsRecord> pRecordNew;
	PendingRecordsType::iterator iterPending = m_PendingRecords.begin();
	for ( ; iterPending != m_PendingRecords.end(); ++iterPending) {
		if ((*iterPending)->m_strHostName == aHostName) {
			QT_WARNING_TRACE_THIS("CQtDnsManager::SyncResolve,"
				" remove pending for hostname=" << aHostName);
			pRecordNew = (*iterPending);
			m_PendingRecords.erase(iterPending);

			// TODO: If it's processing, wait util reloved.
			QT_ASSERTE(pRecordNew->m_State == CQtDnsRecord::RSV_IDLE);
			break;
		}
	}

	int nErr = -998;
	if (!pRecordNew) {
		pRecordNew = new CQtDnsRecord(aHostName
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
			, m_bIsEL98
#endif
			);
		if (!pRecordNew)
			goto fail;
	}
	
	m_PendingRecords.push_front(pRecordNew);
	nErr = DoGetHostByName_l(pRecordNew.ParaIn());
	
fail:
	Resolved_l(pRecordNew.ParaIn(), nErr, FALSE);
	if (!nErr) {
		aRecord = pRecordNew.Get();
		QT_ASSERTE_RETURN(aRecord, QT_ERROR_FAILURE);
		aRecord->AddReference();
		return QT_OK;
	}
	else {
		return QT_ERROR_NETWORK_DNS_FAILURE;
	}
}

QtResult CQtDnsManager::
FindInCache_l(CQtDnsRecord *&aRecord, const CQtString &aHostName)
{
	QT_ASSERTE(!aRecord);

	CacheRecordsType::iterator iter = m_CacheRecords.find(aHostName);
	if (iter != m_CacheRecords.end()) {
		aRecord = (*iter).second.Get();
		QT_ASSERTE(aRecord);
		QT_ASSERTE(aHostName == aRecord->m_strHostName);
		
		if (aRecord->m_State == CQtDnsRecord::RSV_SUCCESS) {
#if defined QT_WIN32 || defined QT_PORT_CLIENT
			QT_INFO_TRACE_THIS("CQtDnsManager::FindInCache_l,find record for host " << aRecord->GetHostName());
#endif
			aRecord->AddReference();
			return QT_OK;
		}
		else if (aRecord->m_State == CQtDnsRecord::RSV_FAILED) {
			aRecord = NULL;
			return QT_ERROR_NETWORK_DNS_FAILURE;
		}
		else {
			QT_WARNING_TRACE_THIS("CQtDnsManager::FindInCache_l,"
				" error state in m_CacheRecords"
				" aHostName=" << aHostName <<
				" aRecord=" << aRecord <<
				" state=" << aRecord->m_State);
			QT_ASSERTE(FALSE);
			return QT_ERROR_UNEXPECTED;
		}
	}
	return QT_ERROR_NOT_FOUND;
}

QtResult CQtDnsManager::RefreshHost(const CQtString &aHostName)
{
	QT_INFO_TRACE_THIS("CQtDnsManager::RefreshHost,"
		" aHostName=" << aHostName);

	CQtComAutoPtr<CQtDnsRecord> pOldRecord;
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	CacheRecordsType::iterator iter = m_CacheRecords.find(aHostName);
	if (iter != m_CacheRecords.end()) {
		pOldRecord = (*iter).second;
		QT_ASSERTE(pOldRecord->m_State == CQtDnsRecord::RSV_SUCCESS || 
			pOldRecord->m_State == CQtDnsRecord::RSV_FAILED);
		QT_ASSERTE(pOldRecord->m_strHostName == aHostName);
		m_CacheRecords.erase(iter);
	}

	if (!pOldRecord) {
		pOldRecord = new CQtDnsRecord(aHostName
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
			, m_bIsEL98
#endif
			);
	}
	else {
		pOldRecord->m_State = CQtDnsRecord::RSV_IDLE;
	}

	int nErr = BeginResolve_l(pOldRecord.ParaIn());
	if (nErr) {
		Resolved_l(pOldRecord.ParaIn(), nErr, FALSE);
		return QT_ERROR_FAILURE;
	}
	return QT_ERROR_WOULD_BLOCK;
}

QtResult CQtDnsManager::GetLocalIps(CQtDnsRecord *&aRecord)
{
	QT_ASSERTE(!aRecord);

	char szBuf[512];
	int nErr = ::gethostname(szBuf, sizeof(szBuf));
	if (nErr != 0) {
#ifdef QT_WIN32
		errno = ::WSAGetLastError();
#endif // QT_WIN32
		QT_WARNING_TRACE_THIS("CQtDnsManager::GetLocalIps, gethostname() failed! err=" << errno);
		return QT_ERROR_FAILURE;
	}

	QtResult rv = SyncResolve(aRecord, szBuf);
	return rv;
}

int CQtDnsManager::BeginResolve_l(CQtDnsRecord *aRecord)
{
	QT_ASSERTE_RETURN(aRecord, -999);

	PendingRecordsType::iterator iterPending = m_PendingRecords.begin();
	for ( ; iterPending != m_PendingRecords.end(); ++iterPending) {
		if ((*iterPending)->m_strHostName == aRecord->m_strHostName) {
			QT_WARNING_TRACE_THIS("CQtDnsManager::BeginResolve_l,"
				" pending for hostname=" << aRecord->m_strHostName);
			return 0;
		}
	}

	CQtComAutoPtr<CQtDnsRecord> pRecordNew = aRecord;
	m_PendingRecords.push_back(pRecordNew);

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	if(m_bIsEL98)
	{
		QtResult rv = QT_OK;
		if (!m_pThreadDNS) 
			rv = SpawnDnsThread_l();
		if (QT_SUCCEEDED(rv))
			rv = m_pThreadDNS->GetEventQueue()->PostEvent(this);
		return QT_SUCCEEDED(rv) ? 0 : -1;
	}
	int nRet = 0;
	// Note form MSDN:
	// The WSAAsyncGetHostByName function is not designed to provide parallel 
	// resolution of several names. Therefore, applications that issue several 
	// requests should not expect them to be executed concurrently.
	//
	// so we have to resolve one by one, and pend it before if resloving.
	if (m_PendingRecords.size() == 1) {
		nRet = DoAsyncHostByName_l(pRecordNew.ParaIn());
	}
	return nRet;
#else
	QtResult rv = QT_OK;
	if (!m_pThreadDNS) 
		rv = SpawnDnsThread_l();
	if (QT_SUCCEEDED(rv))
		rv = m_pThreadDNS->GetEventQueue()->PostEvent(this);
	return QT_SUCCEEDED(rv) ? 0 : -1;
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
}

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
int CQtDnsManager::DoAsyncHostByName_l(CQtDnsRecord *aRecord)
{
	QT_ASSERTE(!m_bIsEL98);
	QT_ASSERTE_RETURN(aRecord, QT_ERROR_FAILURE);
	QT_ASSERTE(aRecord->m_State == CQtDnsRecord::RSV_IDLE);
	aRecord->m_State = CQtDnsRecord::RSV_PROCESSING;

	QT_ASSERTE_RETURN(m_hwndDsnWindow, QT_ERROR_NOT_INITIALIZED);
	HANDLE hdNew = ::WSAAsyncGetHostByName(
		m_hwndDsnWindow, 
		WM_WIN32_DNS_EVENT, 
		aRecord->m_strHostName.c_str(),
		aRecord->m_szBuffer,
		sizeof(aRecord->m_szBuffer));
	if (hdNew == 0) {
		int nErr = ::WSAGetLastError();
		QT_ASSERTE(nErr != 0);
		QT_WARNING_TRACE_THIS("CQtDnsManager::DoAsyncHostByName_l, WSAAsyncGetHostByName() failed."
			" err=" << nErr);
		return nErr;
	}
	QT_ASSERTE(aRecord->m_HandleResolve == 0);
	aRecord->m_HandleResolve = hdNew;
	return 0;
}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

int CQtDnsManager::DoGetHostByName_l(CQtDnsRecord *aRecord)
{
	QT_ASSERTE_RETURN(aRecord, -1);
	QT_ASSERTE(aRecord->m_State == CQtDnsRecord::RSV_IDLE);
	aRecord->m_State = CQtDnsRecord::RSV_PROCESSING;
	::memset(aRecord->m_szBuffer, 0, sizeof(aRecord->m_szBuffer));

	// unlock the mutex because gethostbyname() will block the current thread.
	m_Mutex.UnLock();
	int nError = 0;
	struct hostent *pHeResult = NULL;
	// There will be crash if gethostbyname_r("www.webex.com"), strange!
	// So we have to use gethostbyname() instead gethostbyname_r().
//#ifdef QT_WIN32
#if defined (QT_WIN32)/* || defined (QT_MACOS)*/
	pHeResult = ::gethostbyname(aRecord->m_strHostName.c_str());
	if (pHeResult) {
		CopyHostent_i(aRecord, pHeResult);
	}
	else {
#ifdef QT_WIN32
		nError = ::WSAGetLastError();
#else
		nError = errno;
#endif // QT_WIN32
	}
#else 
	struct hostent *pheResultBuf = 
		reinterpret_cast<struct hostent *>(aRecord->m_szBuffer);
#if defined QT_PORT_CLIENT
// #if (defined QT_SOLARIS/* || defined QT_ENABLE_EPOLL*/) //is linux kernel 2.6 or solaris
	if (::gethostbyname_dns_r(aRecord->m_strHostName.c_str(), 
		pheResultBuf, 
		aRecord->m_szBuffer + sizeof(hostent), 
		sizeof(aRecord->m_szBuffer) - sizeof(hostent), 
		&pheResultBuf,
		&nError) == 0)

/* 

//solaris
		if (::gethostbyname_r(aRecord->m_strHostName.c_str(), 
			pheResultBuf, 
			aRecord->m_szBuffer + sizeof(hostent), 
			sizeof(aRecord->m_szBuffer) - sizeof(hostent), 
			&nError) != NULL)
*/
// #else
// 		if (::gethostbyname_r(aRecord->m_strHostName.c_str(), 
// 			pheResultBuf, 
// 			aRecord->m_szBuffer + sizeof(hostent), 
// 			sizeof(aRecord->m_szBuffer) - sizeof(hostent), 
// 			&pheResultBuf,
// 			&nError) == 0)
// #endif
#else
		if (::gethostbyname_r(aRecord->m_strHostName.c_str(), 
			pheResultBuf, 
			aRecord->m_szBuffer + sizeof(hostent), 
			sizeof(aRecord->m_szBuffer) - sizeof(hostent), 
			&pheResultBuf,
			&nError) == 0)
				
#endif // !QT_SOLARIS

	{
		pHeResult = pheResultBuf;
	}
#endif // QT_WIN32
	if (!pHeResult) {
		if (!nError)
			QT_WARNING_TRACE_THIS("CQtDnsManager::DoGetHostByName_l, gethostbyname() failed."
			" host=" << aRecord->m_strHostName << " err=" << nError);
#ifdef QT_MACOS
		    QT_ASSERTE(nError);
#else
			nError = EADDRNOTAVAIL;
#endif	
	}

	m_Mutex.Lock();
	return nError;
}

static inline void CopyLpstr(LPSTR aSrc, LPSTR &aDst)
{
	// need copy '\0'
	if (aSrc) {
		size_t nLen = strlen(aSrc) + 1;
		::memcpy(aDst, aSrc, nLen);
		aDst += nLen;
	}
	else {
		::memset(aDst, 0, sizeof(LPSTR));
		aDst += sizeof(LPSTR);
	}
}

static inline void Copy4Bytes(DWORD a4BytesBuf, LPSTR &aDst)
{
	::memcpy(aDst, &a4BytesBuf, 4);
	aDst += 4;
}

void CQtDnsManager::
CopyHostent_i(CQtDnsRecord *aRecord, struct hostent *aHostent)
{
	QT_ASSERTE(aRecord);
	QT_ASSERTE(aHostent);

#if 0
	// it will cause crash
	::memcpy(aRecord->m_szBuffer, aHostent, sizeof(aRecord->m_szBuffer));
#else
	struct hostent *pHeNew = reinterpret_cast<struct hostent *>(aRecord->m_szBuffer);
	LPSTR pStart = aRecord->m_szBuffer + sizeof(struct hostent);
	if (aHostent->h_name) {
		pHeNew->h_name = pStart;
		CopyLpstr(aHostent->h_name, pStart);
	}
	else {
		pHeNew->h_name = NULL;
	}
	pHeNew->h_addrtype = aHostent->h_addrtype;
	pHeNew->h_length = aHostent->h_length;

	if (aHostent->h_aliases) {
		pHeNew->h_aliases = reinterpret_cast<char **>(pStart);
		int nNum = 0;
		char **pAliases = aHostent->h_aliases;
		for ( ; *pAliases; ++pAliases) 
			nNum++;

		LPSTR pNumPtr = pStart;
		pStart += (nNum + 1) * 4;
		for (int i = 0; i < nNum; ++i) {
			Copy4Bytes(reinterpret_cast<DWORD>(pStart), pNumPtr);
			CopyLpstr((aHostent->h_aliases)[i], pStart);
		}
		Copy4Bytes(0, pNumPtr);
	}
	else {
		pHeNew->h_aliases = NULL;
	}

	if (aHostent->h_addr_list) {
		pHeNew->h_addr_list = reinterpret_cast<char **>(pStart);
		int nNum = 0;
		char **pAddrList = aHostent->h_addr_list;
		for ( ; *pAddrList; ++pAddrList) 
			nNum++;

		QT_ASSERTE(aHostent->h_length == 4);
		LPSTR pNumPtr = pStart;
		pStart += (nNum + 1) * 4;
		for (int i = 0; i < nNum; ++i) {
			Copy4Bytes(reinterpret_cast<DWORD>(pStart), pNumPtr);
			Copy4Bytes(
				*reinterpret_cast<DWORD*>((aHostent->h_addr_list)[i]), 
				pStart);
		}
		Copy4Bytes(0, pNumPtr);
	}
	else {
		pHeNew->h_addr_list = NULL;
	}
#endif
}

QtResult CQtDnsManager::
TryAddObserver_l(IQtObserver *aObserver, AQtThread *aThreadListener, const CQtString &aHostName)
{
	if (!aObserver)
		return QT_ERROR_INVALID_ARG;

	if (!aThreadListener) {
		aThreadListener = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
		QT_ASSERTE(aThreadListener);
	}

	ObserversType::iterator iter = m_Observers.begin();
	for ( ; iter != m_Observers.end(); ++iter) {
		if ((*iter).m_pObserver == aObserver) {
			QT_WARNING_TRACE_THIS("CQtDnsManager::TryAddObserver_l, observer already exist."
				" aObserver=" << aObserver << 
				" aThreadListener=" << aThreadListener);
			return QT_ERROR_FOUND;
		}
	}

	CObserverAndListener obsNew(this, aObserver, aThreadListener, 0, aHostName);
	m_Observers.push_back(obsNew);
	return QT_OK;
}

QtResult CQtDnsManager::
Resolved_l(CQtDnsRecord *aRecord, int aError, BOOL aCallback)
{
	QT_ASSERTE_RETURN(aRecord, QT_ERROR_FAILURE);
	QT_ASSERTE(aRecord->m_State == CQtDnsRecord::RSV_PROCESSING);
	QT_INFO_TRACE_THIS("CQtDnsManager::Resolved_l,"
		" pRecord=" << aRecord << 
		" hostname=" << aRecord->m_strHostName << 
		" aError=" << aError);
	
	if (!aError) {
		// it's successful.
		aRecord->m_State = CQtDnsRecord::RSV_SUCCESS;
	}
	else {
		aRecord->m_State = CQtDnsRecord::RSV_FAILED;
	}
	aRecord->m_tvResolve = CQtTimeValue::GetTimeOfDay();

	m_CacheRecords[aRecord->m_strHostName] = aRecord;

	PendingRecordsType::iterator iter = std::find(
		m_PendingRecords.begin(), 
		m_PendingRecords.end(), 
		aRecord);
	if (iter != m_PendingRecords.end()) {
		m_PendingRecords.erase(iter);
	}
	else {
		QT_WARNING_TRACE_THIS("CQtDnsManager::Resolved_l, can't find pending."
			" maybe it's removed due to Sync and Aysnc resolve the same hostname."
			" hsotname" << aRecord->m_strHostName);
		QT_ASSERTE(FALSE);
	}

	if (aCallback)
		DoCallback_l(aError, aRecord->m_strHostName);
	return QT_OK;
}

QtResult CQtDnsManager::DoCallback_l(int aError, const CQtString &aHostName)
{
	if (m_Observers.empty())
		return QT_OK;
	ObserversType obvOnCall(m_Observers);

	// don't hold the mutex when doing callback 
	m_Mutex.UnLock();
	ObserversType::iterator iter = obvOnCall.begin();
	for ( ; iter != obvOnCall.end(); ++iter) {
		if ((*iter).m_strHostName != aHostName)
			continue;

		if (CQtThreadManager::IsEqualCurrentThread((*iter).m_pThreadListener->GetThreadId())) {
			IQtObserver *pObserver = (*iter).m_pObserver;
			if (pObserver) {
				// allow OnObserver() once.
				QtResult rv = CancelResolve(pObserver);
				if (QT_SUCCEEDED(rv)) {
					int nErr = aError;
					pObserver->OnObserve("DnsManager", &nErr);
				}
			}
		}
		else {
			IQtEventQueue *pEventQueue = (*iter).m_pThreadListener->GetEventQueue();
			if (pEventQueue) {
				CObserverAndListener *pEventNew = new CObserverAndListener(*iter);
				pEventNew->m_nError = aError;
				pEventQueue->PostEvent(pEventNew);
			}
		}
	}
	m_Mutex.Lock();
	return QT_OK;
}

QtResult CQtDnsManager::CancelResolve(IQtObserver *aObserver)
{
	QT_INFO_TRACE_THIS("CQtDnsManager::CancelResolve,"
		" aObserver=" << aObserver);

	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	ObserversType::iterator iter = m_Observers.begin();
	for ( ; iter != m_Observers.end(); ++iter) {
		if ((*iter).m_pObserver == aObserver) {
			m_Observers.erase(iter);
			return QT_OK;
		}
	}

//	QT_WARNING_TRACE_THIS("CQtDnsManager::CancelResolve,"
//		" observer not exist. aObserver=" << aObserver);
	return QT_ERROR_NOT_FOUND;
}

QtResult CQtDnsManager::Shutdown()
{
	//bug 372965 need add protection to avoid invade gethostbyname  
#if !defined QT_WIN32 && !defined QT_MACOS
	CQtMutexGuardT<MutexType> theShutdownGuard(m_ShutdownMutex);
#endif
	//////////////////////////////////////////////////////////////////////////
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	if(m_bIsEL98) return QT_OK;
	
	if (m_hwndDsnWindow) {
		::DestroyWindow(m_hwndDsnWindow);
		m_hwndDsnWindow = NULL;
	}
#else
	if (m_pThreadDNS) {
		m_pThreadDNS->Stop();
		//m_pThreadDNS->Terminate();
		m_pThreadDNS = NULL;
	}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
	m_Observers.clear();
	m_PendingRecords.clear();
	m_CacheRecords.clear();
	return QT_OK;
}

QtResult CQtDnsManager::CObserverAndListener::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadListener->GetThreadId()));
	
	QtResult rv = m_pDnsManager->CancelResolve(m_pObserver);
	if (QT_SUCCEEDED(rv) && m_pObserver)
		m_pObserver->OnObserve("DnsManager", &m_nError);
	return QT_OK;
}

//#ifndef QT_WIN32_ENABLE_AsyncGetHostByName
QtResult CQtDnsManager::SpawnDnsThread_l()
{
	QT_ASSERTE(!m_pThreadDNS);

	QtResult rv = CQtThreadManager::Instance()->CreateUserTaskThread(
		m_pThreadDNS, 
		CQtThreadManager::TF_JOINABLE, 
		FALSE, CQtThreadManager::TT_DNS);
	if (QT_FAILED(rv)) {
		QT_WARNING_TRACE_THIS("CQtDnsManager::SpawnDnsThread_l, create DNS thread failed!");
	}
	else {
		QT_INFO_TRACE_THIS("CQtDnsManager::SpawnDnsThread_l, create DNS thread.");
	}
	return rv;
}
//#endif // !QT_WIN32_ENABLE_AsyncGetHostByName

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
LRESULT CALLBACK CQtDnsManager::
DnsEventWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) 
	{
	case WM_WIN32_DNS_EVENT:
	{
		CQtDnsManager *pDnsManager = (CQtDnsManager *)::GetWindowLong(hwnd, 0);
		QT_ASSERTE(pDnsManager);

		HANDLE hdFind = (HANDLE)wParam;
		int nError = WSAGETASYNCERROR(lParam);
		CQtMutexGuardT<MutexType> theGuard(pDnsManager->m_Mutex);
		QT_ASSERTE_RETURN(!pDnsManager->m_PendingRecords.empty(), 0);
		CQtDnsRecord *pRecord = (*(pDnsManager->m_PendingRecords.begin())).Get();
		QT_ASSERTE(pRecord->m_HandleResolve == hdFind);
		pRecord->m_HandleResolve = 0;
		pDnsManager->Resolved_l(pRecord, nError);

		while (!pDnsManager->m_PendingRecords.empty()) {
			CQtDnsRecord *pRecordNext = (*pDnsManager->m_PendingRecords.begin()).Get();
			if (pRecordNext->m_State == CQtDnsRecord::RSV_PROCESSING)
				break;

			int nErr = pDnsManager->DoAsyncHostByName_l(pRecordNext);
			if (nErr)
				pDnsManager->Resolved_l(pRecordNext, nErr);
			else
				break;
		}
		return 0;
	}

	default:
		break;
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

QtResult CQtDnsManager::OnEventFire()
{
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	if(!m_bIsEL98)
	{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	return CreateDnsWindow();
	}
	else
	{
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadDNS->GetThreadId()));
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		while (!m_PendingRecords.empty()) {
			// must use CQtComAutoPtr to backup it, because DoGetHostByName_l() 
			// maybe unlock the mutex and it's may remove in other thread.
			CQtComAutoPtr<CQtDnsRecord> pRecord = (*m_PendingRecords.begin());
			int nErr = DoGetHostByName_l(pRecord.ParaIn());
			Resolved_l(pRecord.ParaIn(), nErr);
		}
		return QT_OK;
	}
#else
#if !defined QT_WIN32 && !defined QT_MACOS
	//bug 372965 need add protection to avoid invade gethostbyname  
	CQtMutexGuardT<MutexType> theShutdownGuard(m_ShutdownMutex);
	//////////////////////////////////////////////////////////////////////////
#endif
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	if(!m_pThreadDNS)
	{
		QT_INFO_TRACE_THIS("CQtDnsManager::OnEventFire() DNS thread already stopped!");
		return QT_ERROR_FAILURE;
	}
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadDNS->GetThreadId()));
	while (!m_PendingRecords.empty()) {
		// must use CQtComAutoPtr to backup it, because DoGetHostByName_l() 
		// maybe unlock the mutex and it's may remove in other thread.
		CQtComAutoPtr<CQtDnsRecord> pRecord = (*m_PendingRecords.begin());
		//bug 372965, that function has a unlock operator and lets shutdown function can insert it progress and clear the list
		int nErr = DoGetHostByName_l(pRecord.ParaIn());
		Resolved_l(pRecord.ParaIn(), nErr);
	}
	return QT_OK;
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
}

void CQtDnsManager::OnDestorySelf()
{
	// don't delete this due to singleton.
}

void CQtDnsManager::OnTimer(CQtTimerWrapperID* aId)
{
	if (m_CacheRecords.empty()) 
		return;

	CQtTimeValue tvCurrent = CQtTimeValue::GetTimeOfDay();
	CQtTimeValue tvExpireInterval(3L);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	CacheRecordsType::iterator iter = m_CacheRecords.begin();
	while (iter != m_CacheRecords.end()) {
		CQtDnsRecord *pRecord = (*iter).second.Get();
#if defined QT_WIN32 || defined QT_PORT_CLIENT
		QT_INFO_TRACE_THIS("CQtDnsManager::OnTimer,state=" << pRecord->m_State << ",tvCurrent=" << tvCurrent.GetSec() << ",tvResolve=" << pRecord->m_tvResolve.GetSec());
#endif
		if ((pRecord->m_State == CQtDnsRecord::RSV_SUCCESS || 
			pRecord->m_State == CQtDnsRecord::RSV_FAILED) && 
			(tvCurrent - pRecord->m_tvResolve > tvExpireInterval))
		{
			CacheRecordsType::iterator iterTmp = iter++;
			m_CacheRecords.erase(iterTmp);
		}
		else {
			++iter;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// class CQtDnsRecord
//////////////////////////////////////////////////////////////////////

CQtDnsRecord::CQtDnsRecord(const CQtString &aHostName
#ifdef	   QT_WIN32_ENABLE_AsyncGetHostByName
						   ,BOOL bIsWin98
#endif
						   )
	: m_strHostName(aHostName)
	, m_State(RSV_IDLE)
	, m_tvResolve(CQtTimeValue::GetTimeOfDay())
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	, m_HandleResolve(0)
	, m_bIsEL98(bIsWin98)
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
{
	QT_ASSERTE(!m_strHostName.empty());
	::memset(m_szBuffer, 0, sizeof(m_szBuffer));
}

CQtDnsRecord::~CQtDnsRecord()
{
#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	if(m_bIsEL98)
		return;
	
	if (m_HandleResolve != 0 && m_State == RSV_PROCESSING) {
		::WSACancelAsyncRequest(m_HandleResolve);
		m_HandleResolve = 0;
	}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName
}

