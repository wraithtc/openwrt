/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  ActiveTouch utility header file                                        */
/*                                                                         */
/*  T120TRACE.H                                                               */
/*                                                                         */
/*  Copyright (c) 1997 Stellar Computing Corp.                             */
/*  All rights reserved                                                    */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include "QtBase.h"
#include "QtTraceFromT120.h"

#ifdef QT_QTEC_UNIFIED_TRACE
#if defined QT_WIN32
extern HANDLE g_hInstDll;
#endif
#endif

#ifdef QT_MACOS
static CFBundleRef LoadLibrary(const char* lpszbundle)
{
	// 1.get bundle path
	char cBundlePath[PATH_MAX];
	memset(cBundlePath, 0, PATH_MAX);
	
	Dl_info 	dlInfo;
	static int  sDummy;
	dladdr((void*)&sDummy, &dlInfo);
	
	strlcpy(cBundlePath, dlInfo.dli_fname, PATH_MAX);

	// whether is self a framework ? 
	int locateNumber = 1;
	FSRef bundlePath;
	OSStatus iStatus = FSPathMakeRef((unsigned char*)cBundlePath, &bundlePath, NULL);
	if(noErr == iStatus)
	{
		LSItemInfoRecord  info;
		iStatus = LSCopyItemInfoForRef(&bundlePath, kLSRequestExtension, &info);
		if(noErr == iStatus && NULL == info.extension)
		{
			locateNumber = 4;
		}
	}

	char * pPath = NULL;
	for(int i = 0; i < locateNumber; i++)
	{
		pPath = strrchr(cBundlePath,'/');
		if(pPath)
		{
			*pPath = 0;
		}
		if(NULL == pPath)
			break;
	}
	if(NULL == pPath)
		return NULL;
	
	strlcat(cBundlePath, "/", PATH_MAX);
	strlcat(cBundlePath, lpszbundle, PATH_MAX);
	
	iStatus = FSPathMakeRef((unsigned char*)cBundlePath, &bundlePath, NULL);
	if(noErr != iStatus)
		return NULL;
	
	CFURLRef bundleURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &bundlePath);
	if(NULL == bundleURL)
		return NULL;

	// 2.get bundle ref
	CFBundleRef bundleRef = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
	CFRelease(bundleURL);
	
//	Boolean bReturn = FALSE;
	if(NULL != bundleRef)
	{
	//	bReturn = CFBundleLoadExecutable(bundleRef);
	}
	
	return bundleRef;
}

static Boolean FreeLibrary(CFBundleRef bundle)
{	
	if(NULL != bundle)
	{
	//	CFBundleUnloadExecutable(bundle);
		CFRelease(bundle);
	}
	
	return TRUE;
}

static void* GetProcessAddress(CFBundleRef bundle, const char* lpszprocname)
{
	if(NULL == bundle)
		return NULL;

	CFStringRef cfprocname = CFStringCreateWithCString(NULL,lpszprocname,CFStringGetSystemEncoding());
	void *processAddress = CFBundleGetFunctionPointerForName(bundle,cfprocname);
	CFRelease(cfprocname);
	
	return processAddress;
}
#endif

#ifdef MachOSupport
//#include <stdio.posix.h>
unsigned int MMP_GetCurrentModuleParentPath(char* lpszPath, int length)
{
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	OSErr 	theErr;
	FSSpec 	theSpec;
	
	if (!lpszPath) return 0;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	/* Find the application FSSpec */
	theErr = GetProcessInformation(&thePSN, &theInfo);
	
	if (theErr != noErr)
		return 0;
	
	for(int i = 0;i<4;i++)
	{
		theErr = ::FSMakeFSSpec(theSpec.vRefNum, theSpec.parID, NULL, &theSpec);
		if (theErr != noErr)
			return 0;
	}
	FSRef theRef;
	theErr = ::FSpMakeFSRef(&theSpec,&theRef);
	if (theErr != noErr)
		return 0;
	theErr = ::FSRefMakePath( &theRef, (unsigned char*)lpszPath, length );
	if (theErr != noErr)
		return 0;

	return strlen(lpszPath);
}

#endif

#if (defined (QT_WIN32) && defined (QT_MMP) && !defined QT_OUTPUT_TO_FILE && !defined QT_QTEC_UNIFIED_TRACE)
QTECTRC_IMPLEMENT_MODULE_DYNAMIC(MMP, QTECTRC_MMP_VERSION, 0/*(sizeof(WbxMMPTraceHeader))*/, 64 * 1024);
#endif

#ifdef QT_WIN32
#include <direct.h>
#include <shlobj.h>
#endif // QT_WIN32

void xbase64_init_decode_table();
unsigned long xbase64_calc_encode_buf_size(unsigned char* data, unsigned long len);
unsigned long xbase64_encode(unsigned char* data, unsigned long data_len, 
							 unsigned char* buf, unsigned long buf_len);
unsigned long xbase64_decode(unsigned char* buf, unsigned long buf_len, 
							 unsigned char* data, unsigned long data_len);
unsigned long calculate_tick_interval(unsigned long start, unsigned long end);

#ifdef QT_WIN32
#define get_tick_count GetTickCount
#define output_debug_string OutputDebugString
#else
unsigned long get_tick_count();
void output_debug_string(char* str);
#endif

void init_config(const char* cur_env);
unsigned char get_string_param(char* group, char* item_key, char* item_value, unsigned long len);
int get_int_param(char* group, char* item_key);
unsigned short get_uint16_param(char* group, char* item_key);
unsigned long get_uint32_param(char* group, char* item_key);
unsigned char get_bool_param(char* group, char* item_key, unsigned char default_value);
void set_qtec_home_env(char* home_env);
char* get_qtec_home_dir();
const char* get_process_name();
const char* get_exec_name(void);

unsigned char transport_address_parse(char* transport_address, 
								char* protocol_type, int max_protocol_len, 
								char* host_ip, int max_host_ip_len, unsigned short* port);
char* url_string_encode(char* src, char* dest, int max_len);
unsigned long xml_get_uint32(char* src, char* tag, unsigned long def);
unsigned long xml_get_int32(char* src, char* tag, unsigned long def);
unsigned short xml_get_uint16(char* src, char* tag, unsigned short def);
unsigned short xml_get_int16(char* src, char* tag, unsigned short def);
char* xml_get_string(char* src, char* tag, unsigned short max_length, 
					 char* dest, char* def);
char* get_local_ip();
char* get_local_ip_address();
unsigned char is_ip_address(char* sz);
void resolve_2_ip(char* host_name, char* ip_address);
unsigned long ip_2_id(char* ip_address);
char* id_2_ip(unsigned long ip_address);
void ms_sleep(unsigned long milli_seconds);


#if !defined (QT_WIN32)
  #if !defined QT_MACOS || defined MachOSupport
#include <regex.h>
#include <sys/time.h>
  #endif
#else //!defined (WIN32)
#include <time.h>
#endif //!defined (QT_WIN32)
/////////////////////////////////////////////////////////////////////////////
// functions for regex operations

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif // _MAX_PATH

#if !defined (QT_WIN32) && !defined (QT_MACOS) || defined MachOSupport
#define MAX_REGEX_CNT 100
static regex_t re[MAX_REGEX_CNT];
static int regex_cnt = 0;
//struct tm* localtime_r(const time_t*, struct tm*);

static int regex_load_pattern(char *pattern[], int pattern_cnt)
{
    int i;
	
    /* free previous patterns */
    for (i=0; i<regex_cnt; i++)
        regfree(&re[i]);
	
    regex_cnt = (pattern_cnt > MAX_REGEX_CNT) ? MAX_REGEX_CNT : pattern_cnt;
    for (i=0; i<pattern_cnt; i++)
    {
        if (regcomp(&re[i], pattern[i], REG_EXTENDED|REG_NOSUB) != 0) 
        {
            regex_cnt = 0;   /* no matching in the future, sorry */
            return(-1);      /* report error */
        }
        regex_cnt++;
    }
	
    return(0); /* success */
}

static int regex_match(const char* str)
{
    int i;
    for (i=0; i<regex_cnt; i++) 
    {
        if (regexec(&re[i], str, (size_t) 0, NULL, 0)==0) 
        {   /* match!!! */
            return(0); /* successful match */
        }
    }
    return(-1); /* match failed */
}
#endif // !defined (QT_WIN32) && !defined (QT_MACOS)


#if !defined QT_MACOS || defined MachOSupport
/////////////////////////////////////////////////////////////////////////////
// CQtT120TraceFile class
CQtT120TraceFile::CQtT120TraceFile(const char* pszFileName, unsigned long lMaxSize, unsigned char bShared, unsigned char bEnabled)
{
    m_bShared = bShared;
    m_pFile = NULL;
    m_pFileName = NULL;
    m_lMaxSize = lMaxSize;
	m_nFlushFreq = T120TRACE_DEFAULT_FLUSH_FREQ;
	
	m_buffer = (char*)malloc(m_nFlushFreq*T120TRACE_MAX_TRACE_LEN+1);
	m_pos = 0;
	m_current_line = 0;
	
	m_nCurrentTraceLines = 0;
    m_nCurrentFileId = 0;
	m_bMaxReached = FALSE;
	m_nMagicNumber = 20050704;
	m_nFileHandle = 0;
	
#if !defined (QT_WIN32)
	
    m_lock.l_whence = SEEK_SET;
    m_lock.l_start = 0; 
    m_lock.l_len = 0; 
    m_lock.l_type = F_WRLCK; 
    m_lock.l_pid = 0;
	
    m_unlock.l_whence = SEEK_SET;
    m_unlock.l_start = 0; 
    m_unlock.l_len = 0; 
    m_unlock.l_type = F_UNLCK; 
    m_unlock.l_pid = 0;
#endif //!defined (QT_WIN32)
	
//	m_busy = FALSE;
	
    m_pNext = NULL;
	
    if(pszFileName)
    {
        m_pFileName = new char[strlen(pszFileName) + 1];
		if(m_pFileName)
		{
			strcpy(m_pFileName, pszFileName);
#if !defined (QT_WIN32) && !defined (QT_PORT_CLIENT) //it is server.
			m_pFile = fopen(pszFileName, "w+t");
#elif defined (QT_WIN32) && defined (QT_DEBUG)
			m_pFile = NULL;
#elif defined QT_QTEC_UNIFIED_TRACE  // QTEC Unified Trace don't need to open the file.
			m_pFile = NULL;
#elif defined QT_MACOS && !defined MachOSupport
			m_pFile = NULL;
#elif defined MachOSupport
			char lpsztraceini[255];
			memset(lpsztraceini,0,255);			
			MMP_GetCurrentModuleParentPath(lpsztraceini,255);		
/*
			strcat(lpsztraceini,"/attrace.ini");
			m_pFile = NULL;
			FILE *trCfg = fopen(lpsztraceini, "r");
			if(trCfg)
			{
				char switchBuf[4] = {0};
				if(fread(switchBuf, 1, 1, trCfg) == 1)
				{
					if(switchBuf[0] != '0')
						m_pFile = fopen(pszFileName, "w+t");
				}
				fclose(trCfg);

			}
*/
			m_pFile = fopen(pszFileName, "w+t");
			
#else
			m_pFile = fopen(pszFileName, "w+t");
#endif
			if((m_pFile) && (ftell(m_pFile) == 0))
			{
				long lCurPos = 32;
				fprintf(m_pFile, "%-31ld\n", lCurPos);
				fflush(m_pFile);
			}
		}
	}
	
    if(!m_bShared)
		return;
	
    if(m_pFile)
    {
#if !defined (QT_WIN32)
        chmod(pszFileName, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif //!defined (QT_WIN32)
		
        m_nFileHandle = fileno(m_pFile);
		// SET_CLOSEONEXEC(nFileHandle);
		
#if !defined (QT_WIN32)
        while(fcntl(m_nFileHandle, F_SETLKW, &m_lock) < 0 
            && (errno == EINTR || errno == EAGAIN));
#endif //!defined (WIN32)
		
        fseek(m_pFile, 0, SEEK_END);
        if(ftell(m_pFile) == 0)
        {
            long lCurPos = 32;
            fprintf(m_pFile, "%-31ld\n", lCurPos);
            fflush(m_pFile);
        }
		
#if !defined (QT_WIN32)
        while(fcntl(m_nFileHandle, F_SETLKW, &m_unlock) < 0 
            && (errno == EINTR || errno == EAGAIN));
#endif //!defined (WIN32)
    }
}

CQtT120TraceFile::~CQtT120TraceFile()
{
 //   safe_lock();
	
	flush_buffer();
	
	if (NULL != m_buffer)
	{
		free(m_buffer);
		m_buffer = NULL;
	}
	
    if(NULL != m_pFile)
	{
        fclose(m_pFile);
		m_pFile = NULL;
	}
	
	m_nMagicNumber = 0;
	
//    safe_unlock();
	
    if(m_pFileName)
        delete []m_pFileName;
	
}

/*
unsigned char CQtT120TraceFile::safe_lock()
{
	// if (m_busy || !m_mutex.try_lock())
	//	return FALSE;
	
    m_mutex.Lock();
	m_busy = TRUE;
	return TRUE;
}

void CQtT120TraceFile::safe_unlock()
{
	if (m_busy)
	{
		m_busy = FALSE;
		m_mutex.UnLock();
	}
}
*/

unsigned char CQtT120TraceFile::is_same_file(const char* pszFileName)
{
    if(!pszFileName || !m_pFileName)
        return FALSE;
	
    return strcmp(m_pFileName, pszFileName) == 0;
}

unsigned char CQtT120TraceFile::refresh_settings(unsigned long lMaxSize, int nFreq)
{
 //   if (!safe_lock())
//		return FALSE;
	
	flush_buffer();
	
    m_lMaxSize = lMaxSize;
	if (m_nFlushFreq != nFreq)
	{
		m_nFlushFreq = nFreq;
		
		if (NULL != m_buffer)
			free(m_buffer);
		
		m_buffer = (char*)malloc(m_nFlushFreq*T120TRACE_MAX_TRACE_LEN+1);
		// not necessary because just called flush_buffer
		m_pos = 0;
		m_current_line = 0;
	}
	
//	safe_unlock();
	
	return TRUE;
}

void CQtT120TraceFile::flush_buffer()
{
	// m_pFile is private member, I assume this function will only be 
	// called after won m_mutex 
    if(!m_pFile || (0==m_pos))
        return;
	
    long lCurPos = 0;
    //int nFileHandle = fileno(m_pFile);
	
    fseek(m_pFile, 0, SEEK_SET);
    fscanf(m_pFile, "%ld", &lCurPos);
	
    if(m_lMaxSize > 0 && lCurPos >= (long)m_lMaxSize)
    {
#if !defined (QT_WIN32)
        ftruncate(m_nFileHandle, lCurPos);
#endif //!defined (QT_WIN32)
        fflush(m_pFile);
        //lCurPos = 32;   // reset to the beginning
		m_bMaxReached = TRUE; // set the max reach flag.
    }
    fseek(m_pFile, lCurPos, SEEK_SET);
	
    int nReturn = 0;
	nReturn = fprintf(m_pFile, "%s", m_buffer);
	
    fprintf(m_pFile, "************************* Current Trace Point *************************\n\n");
	
    if(nReturn > 0)
        lCurPos += nReturn;
	
    fseek(m_pFile, 0, SEEK_SET);
    fprintf(m_pFile, "%-31ld\n", lCurPos);
	
    fflush(m_pFile);
    
	m_pos = 0;
	m_current_line = 0;
}

void CQtT120TraceFile::write(const char* lpszModule, 
					   const char* lpszDescription, const char* lpsz)
{
    if(!m_buffer)
        return;
	
#if !defined (QT_WIN32)
    struct timeval timeVal;
    struct tm tmVar;
	
#else //!defined (QT_WIN32)
	struct tm *tmVar;
	time_t timeVal;
#endif //!defined (QT_WIN32)
	
#ifdef __APPEND_MODE__
    if(!m_bShared)
    {
//		if (!safe_lock())
//			return;
		
        if((m_nCurrentTraceLines % 10000) == 0)
        {
            m_nCurrentTraceLines = 0;
            m_nCurrentFileId++;
            if(m_nCurrentFileId >= 10)
                m_nCurrentFileId = 0;
			
            if(m_pFile)
            {
                fflush(m_pFile);
                fclose(m_pFile);
                m_pFile = NULL;
            }
			
			if(m_pFileName)
			{
				char* pTemp = new char[strlen(m_pFileName) + 10];
				if(pTemp)
				{
					sprintf(pTemp, "%s.%1d\0", m_pFileName, m_nCurrentFileId);
					remove(pTemp);
					
					m_pFile = fopen(pTemp, "a+t");
					
#if !defined (QT_WIN32)
					if(m_pFile)
						chmod(pTemp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif //!defined (QT_WIN32)
					
					sprintf(pTemp, "%s.cur\0", m_pFileName);
					FILE* pFileTemp = fopen(pTemp, "wt");
					if(pFileTemp)
					{
						fprintf(pFileTemp, "%d\n", m_nCurrentFileId);
						fclose(pFileTemp);
					}
					
					delete []pTemp;
				}
			}
        }
		
        if(m_pFile)
        {
#if !defined (QT_WIN32)
            gettimeofday(&timeVal, NULL);
            localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
            if(lpszModule && lpszDescription)
            {
                if(regex_match(lpszDescription) != 0)
                    fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s:%s\n", 
					tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
					tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
					timeVal.tv_usec / 1000,
					getpid(), pthread_self(),
					lpszDescription, lpsz);
            }
            else
            {
                if(regex_match(lpszDescription) != 0)
                    fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
					tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
					tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
					timeVal.tv_usec / 1000,
					getpid(), pthread_self(),
					lpsz);
				
            }
#else
			
			time( &timeVal );                /* Get time as long integer. */
			tmVar = localtime( &timeVal ); /* Convert to local time. */
            if(lpszModule && lpszDescription)
            {
                //if(regex_match(lpszDescription) != 0)
				fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s:%s\n", 
					tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
					tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
					0,
					GetCurrentProcessId(), GetCurrentThreadId(),
					lpszDescription, lpsz);
            }
            else
            {
                //if(regex_match(lpszDescription) != 0)
				fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
					tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
					tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
					0,
					GetCurrentProcessId(), GetCurrentThreadId(),
					lpsz);
				
            }
#endif //!defined (QT_WIN32)
			
        }
		
        fflush(m_pFile);
        m_nCurrentTraceLines++;
		
//       safe_unlock();
        return;
    }
#endif
	
	if(m_bShared)
	{
		write_shared(lpszModule, lpszDescription, lpsz);
		return;
	}
	
	// lock trace file object
//    if (!safe_lock())
//		return;
#if !defined (QT_WIN32)
    gettimeofday(&timeVal, NULL);
    localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
	
    int nReturn = 0;
    if(lpszModule && lpszDescription)
    {
        if(regex_match(lpszDescription) != 0)
            nReturn = snprintf(m_buffer+m_pos, m_nFlushFreq*T120TRACE_MAX_TRACE_LEN-m_pos, 
			"[%02d/%02d/%04d %02d:%02d:%02d.%03ld pid=%d tid=%d] %s:%s\n", 
			tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
			tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
			timeVal.tv_usec / 1000,
			getpid(), (int)pthread_self(),
			lpszDescription, lpsz);
    }
    else
    {
        if(regex_match(lpszDescription) != 0)
            nReturn = snprintf(m_buffer+m_pos, m_nFlushFreq*T120TRACE_MAX_TRACE_LEN-m_pos,
			"[%02d/%02d/%04d %02d:%02d:%02d.%03ld pid=%d tid=%d] %s\n", 
			tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
			tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
			timeVal.tv_usec / 1000,
			getpid(), (int)pthread_self(),
			lpsz);
    }
#else //!defined (QT_WIN32)
	// budingc, increase precision
//    time( &timeVal );                /* Get time as long integer. */
//    tmVar = localtime( &timeVal ); /* Convert to local time. */
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
	timeVal = tvCur.GetSec();
	tmVar = localtime(&timeVal);
    int nReturn = 0;
    if(lpszModule && lpszDescription && tmVar)
    {
        //if(regex_match(lpszDescription) != 0)
		nReturn = _snprintf(m_buffer+m_pos, m_nFlushFreq*T120TRACE_MAX_TRACE_LEN-m_pos,
			"[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s:%s\n", 
			tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
			tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
			tvCur.GetUsec()/1000,
			GetCurrentProcessId(), GetCurrentThreadId(),
			lpszDescription, lpsz);
    }
    else
    {
     
        //if(regex_match(lpszDescription) != 0)
		if(tmVar)
		{
		nReturn = _snprintf(m_buffer+m_pos, m_nFlushFreq*T120TRACE_MAX_TRACE_LEN-m_pos, 
			"[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
			tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
			tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
			tvCur.GetUsec()/1000,
			GetCurrentProcessId(), GetCurrentThreadId(),
			lpsz);
		}
		else
		{
			nReturn = _snprintf(m_buffer+m_pos, m_nFlushFreq*T120TRACE_MAX_TRACE_LEN-m_pos, 
				"[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
				0, 0, 1900,
				0, 0, 0,
				tvCur.GetUsec()/1000,
				GetCurrentProcessId(), GetCurrentThreadId(),
				lpsz);
		}
		
    }
#endif //!defined (QT_WIN32)
	
	// budingc modified if file can't open.
    if(nReturn > 0 && m_pFile)
	{
        m_pos += (unsigned long)nReturn;
		QT_ASSERTE(m_pos <= (unsigned long)m_nFlushFreq*T120TRACE_MAX_TRACE_LEN);
		m_current_line++;
	}
	
    if(m_current_line >= m_nFlushFreq)
        flush_buffer();
	
    // unlock trace file object
//    safe_unlock();
	
	return;
}

void CQtT120TraceFile::write_shared(const char* lpszModule, 
							  const char* lpszDescription, 
							  const char* lpsz)
{
#if !defined (QT_WIN32)
    struct timeval timeVal;
    struct tm tmVar;
#else //!defined (QT_WIN32)
	struct tm *tmVar;
	time_t timeVal;
#endif //!defined (QT_WIN32)
    long lCurPos = 0;
	
    if(!lpsz || !m_pFile)
        return;
	
    //int nFileHandle = fileno(m_pFile);
	
//    if (!safe_lock())
//		return;
	
    // Lock trace file
#if !defined (QT_WIN32)
	while(fcntl(m_nFileHandle, F_SETLKW, &m_lock) < 0 
		&& (errno == EINTR || errno == EAGAIN));
#endif //!defined (WIN32)
	
    fseek(m_pFile, 0, SEEK_SET);
    fscanf(m_pFile, "%ld", &lCurPos);
	
    if(m_lMaxSize > 0 && lCurPos >= (long)m_lMaxSize)
    {
#if !defined (QT_WIN32)
        ftruncate(m_nFileHandle, lCurPos);
#endif //!defined (QT_WIN32)
        fflush(m_pFile);
        lCurPos = 32;   // reset to the beginning
    }
    fseek(m_pFile, lCurPos, SEEK_SET);
	
#if !defined (QT_WIN32)
    gettimeofday(&timeVal, NULL);
    localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
	
    int nReturn = 0;
    if(lpszModule && lpszDescription)
    {
        if(regex_match(lpszDescription) != 0)
            nReturn = fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03ld pid=%d tid=%d] %s:%s\n", 
			tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
			tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
			timeVal.tv_usec / 1000,
			getpid(), (int)pthread_self(),
			lpszDescription, lpsz);
    }
    else
    {
        if(regex_match(lpszDescription) != 0)
            nReturn = fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03ld pid=%d tid=%d] %s\n", 
			tmVar.tm_mon + 1, tmVar.tm_mday, tmVar.tm_year + 1900,
			tmVar.tm_hour, tmVar.tm_min, tmVar.tm_sec,
			timeVal.tv_usec / 1000,
			getpid(), (int)pthread_self(),
			lpsz);
    }
#else //!defined (QT_WIN32)
    time( &timeVal );                /* Get time as long integer. */
    tmVar = localtime( &timeVal ); /* Convert to local time. */
    int nReturn = 0;
	if(lpszModule && lpszDescription && tmVar)
    {
        //if(regex_match(lpszDescription) != 0)
		nReturn = fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s:%s\n", 
			tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
			tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
			0,
			GetCurrentProcessId(), GetCurrentThreadId(),
			lpszDescription, lpsz);
    }
    else
    {
        //if(regex_match(lpszDescription) != 0)
		if(tmVar)
		{
		nReturn = fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
			tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
			tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
			0,
			GetCurrentProcessId(), GetCurrentThreadId(),
			lpsz);
		
    }
		else
		{
			nReturn = fprintf(m_pFile, "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s\n", 
				0, 0, 1900,
				0, 0, 0,
				0,
				GetCurrentProcessId(), GetCurrentThreadId(),
				lpsz);
		}
		
    }
#endif //!defined (QT_WIN32)
	
    fprintf(m_pFile, "************************* Current Trace Point *************************\n\n");
	
    if(nReturn > 0)
        lCurPos += nReturn;
	
    fseek(m_pFile, 0, SEEK_SET);
    fprintf(m_pFile, "%-31ld\n", lCurPos);
	
    fflush(m_pFile);
    
    // UnLock trace file
#if !defined (QT_WIN32)
    while(fcntl(m_nFileHandle, F_SETLKW, &m_unlock) < 0 
        && (errno == EINTR || errno == EAGAIN));
#endif //!defined (WIN32)
//    safe_unlock();
}


/////////////////////////////////////////////////////////////////////////////
// CQtT120TraceFileMgr class
CQtT120TraceFileMgr::CQtT120TraceFileMgr()
{
    m_cCount = 0;
    m_pHead = NULL;
}

CQtT120TraceFileMgr::~CQtT120TraceFileMgr()
{
#if !defined (QT_WIN32)
	ms_sleep(200);
#else //!defined (QT_WIN32)
	Sleep(200);
#endif //!defined (QT_WIN32)
    while(m_pHead)
    {
        CQtT120TraceFile* pTraceFile = m_pHead;
        m_pHead = m_pHead->m_pNext;
        delete pTraceFile;
    }
	
	//    init_reg_filter();
}

void CQtT120TraceFileMgr::init_reg_filter()
{
#if !defined (QT_WIN32)
    char* pHomeDir = get_qtec_home_dir();
	
    char achFilterFilePath[_MAX_PATH];
    strcpy(achFilterFilePath, pHomeDir);
    if(achFilterFilePath[strlen(achFilterFilePath) - 1] != '/')
        strcat(achFilterFilePath, "/");
    strcat(achFilterFilePath, "conf/qtecexp.filter");
	
    char* pchPatterns[MAX_REGEX_CNT];
    char achLineBuf[1024];
    int i = 0;
    int cPattenCount = 0;
	
    for(i = 0; i < MAX_REGEX_CNT; i++)
        pchPatterns[i] = NULL;
	
    FILE* f = fopen(achFilterFilePath, "rt");
    if(f)
    {
        for(i = 0; i < MAX_REGEX_CNT; i++)
        {
            if(feof(f))
                break;
			
            if(!fgets(achLineBuf, sizeof(achLineBuf), f))
                break;
			
            pchPatterns[cPattenCount] = new char[strlen(achLineBuf) + 1];
            strcpy(pchPatterns[cPattenCount], achLineBuf);
            cPattenCount++;
        }
		
        fclose(f);
    }
	
    if(cPattenCount > 0)
        regex_load_pattern(pchPatterns, cPattenCount);
	
    for(i = 0; i < MAX_REGEX_CNT; i++)
        if(pchPatterns[i])
            delete []pchPatterns[i];
#endif //!defined (QT_WIN32)
}

void CQtT120TraceFileMgr::close(void* hTrace)
{
	m_mutex.Lock();
	CQtT120TraceFile* pPrevTraceFile = NULL;
	CQtT120TraceFile* pCurrTraceFile = NULL;
	CQtT120TraceFile* pNextTraceFile = NULL;
	pCurrTraceFile = m_pHead;
	while(pCurrTraceFile)
    {
        pNextTraceFile = pCurrTraceFile->m_pNext;
		if (pCurrTraceFile == hTrace)
		{
			if (m_pHead == pCurrTraceFile)
				m_pHead = pCurrTraceFile->m_pNext;
			delete pCurrTraceFile;
			if (pPrevTraceFile)
			{
				pPrevTraceFile->m_pNext = pNextTraceFile;
			}
			break;
		} 
		else 
		{
			pPrevTraceFile = pCurrTraceFile;
			pCurrTraceFile = pNextTraceFile;
		}
        
    }
    m_mutex.UnLock();
	
}

void* CQtT120TraceFileMgr::open(const char* pszFileName, unsigned long lMaxSize, unsigned char bShared, unsigned char bEnabled)
{
    m_mutex.Lock();
	
    CQtT120TraceFile* pTraceFile = NULL;
	
    CQtT120TraceFile* p = m_pHead;
    while(p)
    { 
        if(p->is_same_file(pszFileName))
        {
            pTraceFile = p;
            break;
        }
        p = p->m_pNext;
    }
	
    if(!pTraceFile)
    {
        pTraceFile = new CQtT120TraceFile(pszFileName, lMaxSize, bShared, bEnabled);
        if(pTraceFile)
        {
            m_cCount++;
            pTraceFile->m_pNext = m_pHead;
            m_pHead = pTraceFile;
        }
    }
	
    m_mutex.UnLock();
	
    return (void*)pTraceFile;
}

void CQtT120TraceFileMgr::write(void* hHandle, const char* lpszModule, 
						   const char* lpszDescription, 
						   const char* lpsz)
{
    CQtT120TraceFile* pTraceFile = (CQtT120TraceFile*)hHandle;
    if(pTraceFile)
        pTraceFile->write(lpszModule, lpszDescription, lpsz);
}

void CQtT120TraceFileMgr::refresh_settings(unsigned long lMaxSize, int nFreq)
{
    CQtT120TraceFile* p = m_pHead;
    while(p)
    { 
		p->refresh_settings(lMaxSize, nFreq);
        p = p->m_pNext;
    }
}

/////////////////////////////////////////////////////////////////////////////
// global object and functions
CQtT120TraceFileMgr g_trace_mgr;

void* T120_Open_Trace_Dev(const char* pszFileName, unsigned long lMaxSize, unsigned char bShared, unsigned char bEnabled)
{
    return g_trace_mgr.open(pszFileName, lMaxSize, bShared, bEnabled);
}

void T120_Close_Trace_Dev(void * pTrace)
{
    g_trace_mgr.close(pTrace);
}


void T120_Write_Trace_Dev(void* hHandle, const char* lpszModule, 
						  const char* lpszDescription, 
						  const char* lpsz)
{
    g_trace_mgr.write(hHandle, lpszModule, lpszDescription, lpsz);
}

void T120_Refresh_Settings(unsigned long lMaxSize, int nFreq)
{
	g_trace_mgr.refresh_settings( lMaxSize, nFreq);
}

/////////////////////////////////////////////////////////////////////////////
// CQtT120Trace
// This class is used for supporting the trace macros
//
CQtT120Trace* CQtT120Trace::t120_trace = NULL;

CQtT120Trace::CQtT120Trace(char* lpszModule, CQtT120TraceMapEntry mapEntries[],
					   char* lpszPrivateInfo, unsigned char bShared)
{
    m_lpszPrivateInfo = lpszPrivateInfo;
    m_lpszModule = lpszModule;
    m_pMapEntries = mapEntries;
	m_nMapEntries = 26;
	
    m_hTrace = NULL;
	m_hTraceWarning = NULL;
	m_hTraceError = NULL;
    m_hTrace_log = NULL;
    m_bShared = bShared;
    m_bEnabled = TRUE;
	m_bMultiLogs = FALSE;
	m_iCurrCount = 0;
	m_iWarningCount = 0;
	m_iErrorCount = 0;
	m_iCurrInfoDay = 0;
	m_iCurrInfoMonth = 0;
#if !defined (WIN32)
	struct timeval timeVal;
	struct tm tmVar;
	gettimeofday(&timeVal, NULL);
    localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
	m_iCurrInfoDay = tmVar.tm_mday;
	m_iCurrInfoMonth = tmVar.tm_mon + 1;
	m_processid = getpid();
#else //!defined (WIN32)
	struct tm *tmVar;
	time_t timeVal;
	time( &timeVal );                /* Get time as long integer. */
    tmVar = localtime( &timeVal ); /* Convert to local time. */
	if(tmVar)
	{
	m_iCurrInfoDay = tmVar->tm_mday;
	m_iCurrInfoMonth = tmVar->tm_mon + 1;
	}
	else
	{
		m_iCurrInfoDay = 0;
		m_iCurrInfoMonth = 0;

	}
	m_processid = GetCurrentProcessId();
#endif //!defined (WIN32)
    load();
#if defined QT_QTEC_UNIFIED_TRACE
	m_hTraceHandle = NULL;
	m_fpDebugTrace = NULL;
	m_fpInfoTrace = NULL;
	m_fpWarnTrace = NULL;
	m_fpErrorTrace = NULL;
#if defined QT_WIN32
	CHAR achPath[ _MAX_PATH];
	GetModuleFileName( ( HMODULE)g_hInstDll, achPath, _MAX_PATH);
	CQtString cmPath = achPath;
	int nPos = cmPath.rfind( '\\', cmPath.size());
	cmPath.resize( nPos);
	cmPath.append( "\\wbxtrace.dll");
	m_hTraceHandle = ::LoadLibrary( cmPath.c_str( ));
	if( m_hTraceHandle) {
		m_fpDebugTrace = ( QT_QTEC_TRACE)::GetProcAddress( ( HMODULE)m_hTraceHandle, "QTECDEBUGA");
		m_fpInfoTrace = ( QT_QTEC_TRACE)::GetProcAddress( ( HMODULE)m_hTraceHandle, "QTECINFOA");
		m_fpWarnTrace = ( QT_QTEC_TRACE)::GetProcAddress( ( HMODULE)m_hTraceHandle, "QTECWARNA");
		m_fpErrorTrace = ( QT_QTEC_TRACE)::GetProcAddress( ( HMODULE)m_hTraceHandle, "QTECERRORA");
	}
#elif defined QT_MACOS
	m_hTraceHandle = LoadLibrary("wbxtrace.bundle");
	if(m_hTraceHandle) {
		m_fpDebugTrace = ( QT_QTEC_TRACE2)GetProcessAddress( (CFBundleRef)m_hTraceHandle, "QTECDEBUG2");
		m_fpInfoTrace = ( QT_QTEC_TRACE2)GetProcessAddress( (CFBundleRef)m_hTraceHandle, "QTECINFO2");
		m_fpWarnTrace = ( QT_QTEC_TRACE2)GetProcessAddress( (CFBundleRef)m_hTraceHandle, "QTECWARN2");
		m_fpErrorTrace = ( QT_QTEC_TRACE2)GetProcessAddress( (CFBundleRef)m_hTraceHandle, "QTECERROR2");
	}
#elif defined QT_LINUX || defined QT_SOLARIS || defined QT_UNIX
	CQtString	cmPath;
	Dl_info		DlInfo;
	static int	nMmTPAddress;
    dladdr( &nMmTPAddress, &DlInfo);
	cmPath = DlInfo.dli_fname;
	int nPos = cmPath.rfind('/', cmPath.size());
	cmPath.resize(nPos);
	cmPath.append("/libwbxtrace.so");

	m_hTraceHandle = dlopen( cmPath.c_str(), RTLD_LAZY);
	if (m_hTraceHandle) {
		m_fpDebugTrace = ( QT_QTEC_TRACE2)::GetProcAddress( m_hTraceHandle, "QTECDEBUG2");
		m_fpInfoTrace = ( QT_QTEC_TRACE2)::GetProcAddress( m_hTraceHandle, "QTECINFO2");
		m_fpWarnTrace = ( QT_QTEC_TRACE2)::GetProcAddress( m_hTraceHandle, "QTECWARN2");
		m_fpErrorTrace = ( QT_QTEC_TRACE2)::GetProcAddress( m_hTraceHandle, "QTECERROR2");
	}
#endif

#endif
}

#if defined QT_QTEC_UNIFIED_TRACE //for bug303444, need unload wbxtrace.dll when unload MMP
void CQtT120Trace::Close()
{
#if defined QT_WIN32
	if( m_hTraceHandle) {
		::FreeLibrary( ( HMODULE)m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
	}
#elif defined QT_MACOS
	if (m_hTraceHandle) {
		FreeLibrary( (CFBundleRef)m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
		m_fpErrorTrace = NULL;
	}
#elif defined QT_LINUX || defined QT_SOLARIS || defined QT_UNIX
	if (m_hTraceHandle) {
		::FreeLibrary( m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
		m_fpErrorTrace = NULL;
	}
#endif
	m_hTraceHandle = NULL;
}
#endif

CQtT120Trace::~CQtT120Trace()
{
    // don't need to close, trace manager will automatically
    // close all the trace files
    m_hTrace = NULL;
    m_hTrace_log = NULL;
#if defined QT_QTEC_UNIFIED_TRACE

#if defined QT_WIN32
	if( m_hTraceHandle) {
		::FreeLibrary( ( HMODULE)m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
		m_fpErrorTrace = NULL;
	}
#elif defined QT_MACOS
	if (m_hTraceHandle) {
		FreeLibrary( (CFBundleRef)m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
		m_fpErrorTrace = NULL;
	}
#elif defined QT_LINUX || defined QT_SOLARIS || defined QT_UNIX
	if (m_hTraceHandle) {
		::FreeLibrary( m_hTraceHandle);
		m_fpDebugTrace = NULL;
		m_fpInfoTrace = NULL;
		m_fpWarnTrace = NULL;
		m_fpErrorTrace = NULL;
	}
#endif

#endif

}

CQtT120Trace* CQtT120Trace::instance()
{
	if(!t120_trace)
	{
		static CQtT120TraceMapEntry g_logMapEntry[] = 
		{
			{ 0, "Error",      TRUE  },
				{ 1, "Server",      TRUE  },
				{ 2, "NA",          FALSE  },
				{ 3, "NA",          FALSE  },
				{ 4, "NA",          FALSE  },
				{ 5, "NA",          FALSE  },
				{ 6, "NA",          FALSE  },
				{ 7, "NA",          FALSE  },
				{ 8, "NA",          FALSE  },
				{ 9, "NA",          FALSE  },
				{ 10, "Warning",    TRUE  },
				{ 11, "Meeting",    TRUE  },
				{ 12, "Session",    TRUE  },
				{ 13, "User",       TRUE  },
				{ 14, "Statistic",  TRUE  },
				{ 15, "NA",         FALSE  },
				{ 16, "NA",         FALSE  },
				{ 17, "NA",         FALSE  },
				{ 18, "NA",         FALSE  },
				{ 19, "NA",         FALSE  },
				{ 20, "Info",	    TRUE  },
				{ 21, "State",      TRUE  },
				{ 22, "PDU",        FALSE },
				{ 23, "Func",       FALSE },
				{ 24, "Tick",       FALSE },
				{ 25, "Detail",     FALSE }

		};
		
		t120_trace = new CQtT120Trace("Log", g_logMapEntry, NULL, FALSE);
	}
	
	return t120_trace;
}

void CQtT120Trace::read_config()
{
	CQtMutexGuardT<CQtMutexThreadRecursive> theLock(m_mutex);

//    long lMaxTraceFileSize = T120TRACE_DEFAULT_TRACE_SIZE;
	
	int nFreq = get_int_param("Trace", "FlushPerLines");
	if (nFreq<1)
		nFreq = T120TRACE_DEFAULT_FLUSH_FREQ;
	
	int nSize = get_int_param("Trace", (char*)get_process_name());
    if(nSize <= 1024)
        nSize = T120TRACE_DEFAULT_TRACE_SIZE;
	
	T120_Refresh_Settings(nSize, nFreq);
	
    m_bEnabled = get_bool_param("Trace", "Enable", TRUE);
	
	//
	//	Load trace map entries for each process from qtec.cfg. example: 
	//	[Trace]
	//	XXXX.Error=FALSE;
	//	XXXX.Info=FALSE;
	//	XXXX.PDU=TRUE;
	//
    CQtT120TraceMapEntry* pEntry = m_pMapEntries;
	for(unsigned long i = 0; i < m_nMapEntries; i++)
	{
		if(pEntry && pEntry->m_lpszMaskDescription != NULL)
		{
			char achTemp[128];
			sprintf(achTemp, "%s.%s", get_process_name(), pEntry->m_lpszMaskDescription);
			
			if(pEntry->m_bEnable)
				pEntry->m_bEnable = get_bool_param("Trace", achTemp, TRUE);
			else
				pEntry->m_bEnable = get_bool_param("Trace", achTemp, FALSE);
			
			pEntry++;
		}
	}
}

unsigned char CQtT120Trace::load(void * hTrace)
{
	CQtMutexGuardT<CQtMutexThreadRecursive> theLock(m_mutex);

    char achFileName[512];
	char achFileNameWarning[512];
	char achFileNameError[512];
	achFileName[0] = '\0';
	achFileNameWarning[0] = '\0';
	achFileNameError[0] = '\0';

    long lMaxTraceFileSize = 20*1024*1024L;
#if !defined (QT_WIN32)
    struct timeval timeVal;
    struct tm tmVar;
#else //!defined (QT_WIN32)
	struct tm *tmVar;
	time_t timeVal;
#endif //!defined (QT_WIN32)	
    if(hTrace == NULL && m_hTrace) 
	{
        return TRUE;
	}

	// budingc modified to ensure file can open.
	// '0777' allows all users read and write.
#ifndef QT_WIN32
	::mkdir(get_trace_dir(), 0777);
#endif // !QT_WIN32
	
#if !defined (QT_WIN32)
    gettimeofday(&timeVal, NULL);
    localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
#else //!defined (QT_WIN32)
    time( &timeVal );                /* Get time as long integer. */
    tmVar = localtime( &timeVal ); /* Convert to local time. */
#endif //!defined (QT_WIN32)
    if(m_lpszPrivateInfo)
    {
        if(m_bShared)
        {
            snprintf(achFileName,sizeof(achFileName), "%s/%s", 
                get_trace_dir(), m_lpszPrivateInfo);
        }
        else
        {
#if !defined (QT_WIN32)
            snprintf(achFileName,sizeof(achFileName), "%s/%s.%d", 
                get_trace_dir(), m_lpszPrivateInfo, getpid());
#else //!defined (QT_WIN32)
            snprintf(achFileName,sizeof(achFileName), "%s/%s.%d", 
                get_trace_dir(), m_lpszPrivateInfo, GetCurrentProcessId());
#endif //!defined (QT_WIN32)
        }
		
        int nSize = get_int_param("Trace", m_lpszPrivateInfo);
        if(nSize > 1024)
            lMaxTraceFileSize = nSize;
    }
    else
    {
#if !defined (QT_WIN32) && !defined (QT_PORT_CLIENT)
        //sprintf(achFileName, "%s/%s%02d%02d.%d", get_trace_dir(), get_process_name(), tmVar.tm_mon + 1, tmVar.tm_mday, 
        //    getpid());
		if (m_iCurrInfoDay != tmVar.tm_mday || m_iCurrInfoMonth != tmVar.tm_mon + 1)
		{
			m_iCurrInfoDay = tmVar.tm_mday;
			m_iCurrInfoMonth = tmVar.tm_mon + 1;
			m_iCurrCount = 0;
			m_iWarningCount = 0;
			m_iErrorCount = 0;
		}

		if (hTrace == m_hTrace || hTrace == NULL)
		{
			snprintf(achFileName,sizeof(achFileName), "%s/%s_info_%02d%02d%04d_%d.%d.log", 
				get_trace_dir(), get_process_name(), tmVar.tm_mon + 1, tmVar.tm_mday,
				tmVar.tm_year + 1900, m_iCurrCount, m_processid);
			m_iCurrCount++;
			if (m_iCurrCount > 9)
				m_iCurrCount = 0;
		}
/*		if (hTrace == m_hTraceWarning || hTrace == NULL)
		{
			sprintf(achFileNameWarning, "%s/%s_warning_%02d%02d%04d_%d.%d.log", 
				get_trace_dir(), get_process_name(), tmVar.tm_mon + 1, tmVar.tm_mday,
				tmVar.tm_year + 1900, m_iWarningCount, m_processid);
			m_iWarningCount++;
			if (m_iWarningCount > 9)
				m_iWarningCount = 0;
		}
*/
		if (hTrace == m_hTraceError || hTrace == NULL)
		{
			sprintf(achFileNameError, "%s/%s_error_%02d%02d%04d_%d.%d.log", 
				get_trace_dir(), get_process_name(), tmVar.tm_mon + 1, tmVar.tm_mday,
				tmVar.tm_year + 1900, m_iErrorCount, m_processid);
			m_iErrorCount++;
			if (m_iErrorCount > 9)
				m_iErrorCount = 0;
		}

#else //!defined (QT_WIN32)
//        sprintf(achFileName, "%s/%s%02d%02d.%d", get_trace_dir(), get_process_name(), tmVar->tm_mon + 1, tmVar->tm_mday, 
//            GetCurrentProcessId());
#if defined QT_EXPORT_TRACE_FOR_EACH_PROCESS
		CQtString strFileName;
		strFileName += get_trace_dir();
		strFileName += "/";
		strFileName += get_process_name();
		strFileName += "_Trace_";
		char szProcessID[128] = {0};
#ifdef QT_WIN32
		xtoa_wbx<DWORD>(::GetCurrentProcessId(), szProcessID, sizeof(szProcessID));
#else
		xtoa_wbx<int>(m_processid, szProcessID, sizeof(szProcessID));
#endif
		strFileName += szProcessID;
		strFileName += ".txt";
		memcpy(achFileName, strFileName.c_str(), (strFileName.length() + 1) > 512 ? 511 : strFileName.length() + 1);		
		achFileName[511] = 0;
#else
		snprintf(achFileName, sizeof(achFileName),"%s/%s_Trace.txt", get_trace_dir(), get_process_name());
#endif
#endif //defined (QT_WIN32)
		
		int nSize = get_int_param("Trace", (char*)get_process_name());
        // int nSize = get_int_param("Trace", "This");
        if(nSize > 1024)
            lMaxTraceFileSize = nSize;
    }
	
	// budingc, support enabled.
	m_bEnabled = get_bool_param("Trace", "Enable", TRUE);

//    m_hTrace = T120_Open_Trace_Dev(achFileName, lMaxTraceFileSize, m_bShared);
	
	if (hTrace!= NULL)
		T120_Close_Trace_Dev (hTrace);
	if (achFileName[0] && (hTrace == m_hTrace || hTrace == NULL))
		m_hTrace = T120_Open_Trace_Dev(achFileName, lMaxTraceFileSize, m_bShared, m_bEnabled);

	if (achFileNameWarning[0] && (hTrace == m_hTraceWarning || hTrace == NULL))
		m_hTraceWarning = T120_Open_Trace_Dev(achFileNameWarning, lMaxTraceFileSize, m_bShared, m_bEnabled);
	if (achFileNameError[0] && (hTrace == m_hTraceError || hTrace == NULL))
		m_hTraceError = T120_Open_Trace_Dev(achFileNameError, lMaxTraceFileSize, m_bShared, m_bEnabled);
	
	/*	Disable log file for each process
    sprintf(achFileName, "%s.log", achFileName);
    m_hTrace_log = T120_Open_Trace_Dev(achFileName, lMaxTraceFileSize, m_bShared);
	*/
	
	read_config();

	
    return TRUE;
}

const char* CQtT120Trace::get_trace_dir()
{
    static char s_achTraceDir[512];
#ifndef QT_WIN32
    if(get_qtec_home_dir())
    {
        sprintf(s_achTraceDir, "%s/logs", get_qtec_home_dir());
        return s_achTraceDir;
    }
#else 
//	if (::GetModuleFileNameA(NULL, s_achTraceDir, sizeof(s_achTraceDir))) {
/*	if (::SHGetSpecialFolderPath(NULL, s_achTraceDir, CSIDL_APPDATA, FALSE)) {
		int ch = '\\';
		char* pEnd = ::strrchr(s_achTraceDir, ch);
		if (pEnd) {
			*pEnd = '\0';
			return s_achTraceDir;
		}
	}
*/
	char *szUserPath = getenv("USERPROFILE");
	if(!szUserPath)
		szUserPath = getenv("TEMP");
	if(szUserPath)
	{
		strncpy(s_achTraceDir, szUserPath, sizeof(s_achTraceDir));
		return s_achTraceDir;
	}
	return "C:\\TEMP"; 
#endif // !QT_WIN32
    return "/tmp";
}

#ifdef QT_WIN32 
#define T120_OS_SERPATOR '\\'
#else
#define T120_OS_SERPATOR '/'
#endif // QT_WIN32

const char* CQtT120Trace::get_process_name()
{
    const char* pExecName = get_exec_name();
	
    if(pExecName)
    {
        const char* pch = pExecName + strlen(pExecName) - 1;
        while(pch > pExecName && *pch != T120_OS_SERPATOR)
            pch--;
		
        if(*pch == T120_OS_SERPATOR)
            pch++;
		
        return pch;
    }
    else
    {
        return "attrace";
    }
}

/////////////////////////////////////////////////////////////////////////////
// Inner class CQtT120Trace::Text_Formator
CQtT120Trace::Text_Formator::Text_Formator(char* lpszBuf, unsigned long dwBufSize)
{
    m_lpszBuf = lpszBuf;
    m_dwSize  = dwBufSize;
	
    reset();
}
CQtT120Trace::Text_Formator::~Text_Formator()
{
}

void CQtT120Trace::Text_Formator::reset()
{
    m_dwPos   = 0;
    m_bHex    = FALSE;
	
    memset(m_lpszBuf, 0, m_dwSize);
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (char ch)
{
    return *this << (int)ch;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (unsigned char ch)
{
    return *this << (int)ch;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (short s)
{
    return *this << (int)s;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (unsigned short s)
{
    return *this << (int)s;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (_int64 ll)
{
	char buff[65];
	xtoa_wbx<_int64>(ll, buff, sizeof(buff));
	return *this << buff;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (int i)
{
    char achBuf[80];
	
    if(!get_hex_flag())
        sprintf(achBuf, "%d", i);
    else
        sprintf(achBuf, "%x", i);
	
    advance(achBuf);
    set_hex_flag(FALSE);
	
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (unsigned int i)
{
    char achBuf[80];
	
    if(!get_hex_flag())
        sprintf(achBuf, "%u", i);
    else
        sprintf(achBuf, "%x", i);
	
    advance(achBuf);
    set_hex_flag(FALSE);
	
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (long l)
{
    char achBuf[80];
	
    if(!get_hex_flag())
        sprintf(achBuf, "%ld", l);
    else
        sprintf(achBuf, "%lx", l);
	
    advance(achBuf);
    set_hex_flag(FALSE);
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (unsigned long l)
{
    char achBuf[80];
	
    if(!get_hex_flag())
        sprintf(achBuf, "%lu", l);
    else
        sprintf(achBuf, "%lx", l);
	
    advance(achBuf);
    set_hex_flag(FALSE);
	
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (float f)
{
    char achBuf[80];
    sprintf(achBuf, "%f", f);
	
    advance(achBuf);
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (double d)
{
    char achBuf[80];
	
    sprintf(achBuf, "%f", d);
	
    advance(achBuf);
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (const char* lpsz)
{
    advance(lpsz);
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (const CQtString& str)
{
    return (*this << str.c_str());
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (void* lpv)
{
    *this << "0x" << hex << (unsigned long)lpv;
    return *this;
}

CQtT120Trace::Text_Formator& CQtT120Trace::Text_Formator::operator << (Ordix ordix)
{
    switch(ordix)
    {
    case hex :
        set_hex_flag(TRUE);
        break;
		
    case decimal :
        set_hex_flag(FALSE);
        break;
		
    default :
        break;
    }
    return *this;
}

CQtT120Trace::Text_Formator::operator char*()
{
    return m_lpszBuf;
}

void CQtT120Trace::Text_Formator::set_hex_flag(unsigned char bValue)
{
    m_bHex = bValue;
}

unsigned char CQtT120Trace::Text_Formator::get_hex_flag()
{
    return m_bHex;
}

void CQtT120Trace::Text_Formator::advance(const char* lpsz)
{
    if(lpsz)
    {
        unsigned long nLength = (unsigned long)strlen(lpsz);
        if(nLength > m_dwSize - m_dwPos - 64)
            nLength = m_dwSize - m_dwPos - 64;
		
        if(nLength > 0)
        {
            memcpy(m_lpszBuf + m_dwPos*sizeof(char), lpsz, 
                nLength*sizeof(char));
            m_dwPos += nLength;
        }
    }
}

// API methods
void CQtT120Trace::trace_string(unsigned long dwMask, char* lpsz)
{
    if(m_hTrace && m_bEnabled && m_pMapEntries[dwMask].m_bEnable)
    {
		/*
#if defined (QT_WIN32) && defined (QT_DEBUG)
		char szBuf[T120TRACE_MAX_TRACE_LEN];
		struct tm *tmVar;
		time_t timeVal;
		szBuf[0] = '\0';
		// budingc, increase precision
		CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
		timeVal = tvCur.GetSec();
		tmVar = localtime(&timeVal);
		if(m_lpszModule && m_pMapEntries[dwMask].m_lpszMaskDescription) {
			snprintf(szBuf, sizeof(szBuf), "[%02d/%02d/%04d %02d:%02d:%02d.%03d pid=%d tid=%d] %s:%s\n", 
					tmVar->tm_mon + 1, tmVar->tm_mday, tmVar->tm_year + 1900,
					tmVar->tm_hour, tmVar->tm_min, tmVar->tm_sec,
					tvCur.GetUsec() / 1000,
					GetCurrentProcessId(), GetCurrentThreadId(),
					m_pMapEntries[dwMask].m_lpszMaskDescription, lpsz);
        }
		::OutputDebugStringA(szBuf);
		*/
#if defined QT_QTEC_UNIFIED_TRACE && defined QT_WIN32
		switch( m_pMapEntries[ dwMask].m_dwMask) {
		case 0:
			//QTECERRORA( "%s", lpsz);
			if( m_fpErrorTrace)
				m_fpErrorTrace( "%s", lpsz);
			break;
		case 10:
			//QTECWARNA( "%s", lpsz);
			if( m_fpWarnTrace) 
				m_fpWarnTrace( "%s", lpsz);
			break;
		case 20:
			//QTECINFOA( "%s", lpsz);
			if( m_fpInfoTrace)
				m_fpInfoTrace( "%s", lpsz);
			break;
		}
		return;
#elif defined QT_QTEC_UNIFIED_TRACE
#if defined QT_UNIX || defined QT_LINUX || defined QT_SOLARIS
		CHAR szModule[ 12] = "UNIX";
#elif defined QT_MACOS
		CHAR szModule[ 12] = "MAC";
#endif
		switch( m_pMapEntries[ dwMask].m_dwMask) {
		case 0:
			if( m_fpErrorTrace)
				m_fpErrorTrace( szModule, "%s", lpsz);
			break;
		case 10:
			if( m_fpWarnTrace) 
				m_fpWarnTrace( szModule, "%s", lpsz);
			break;
		case 20:
			if( m_fpInfoTrace)
				m_fpInfoTrace( szModule, "%s", lpsz);
			break;
		}
		return;
#else // defined (QT_WIN32) && defined (QT_DEBUG)

		CQtMutexGuardT<CQtMutexThreadRecursive> theLock(m_mutex);

        T120_Write_Trace_Dev(m_hTrace, m_lpszModule,
            m_pMapEntries[dwMask].m_lpszMaskDescription,
            lpsz);

		struct tm tmVar;
#ifndef QT_WIN32
		struct timeval timeVal;
		gettimeofday(&timeVal, NULL);
		localtime_r((const time_t*)&timeVal.tv_sec, &tmVar);
#else
		time_t timeVal;
		time( &timeVal );              /* Get time as long integer.*/
		tmVar = *localtime( &timeVal ); /* Convert to local time. */
#endif
		if (m_iCurrInfoDay != tmVar.tm_mday || m_iCurrInfoMonth != tmVar.tm_mon + 1)
		{
			m_iCurrInfoDay = tmVar.tm_mday;
			m_iCurrInfoMonth = tmVar.tm_mon + 1;
			m_iCurrCount = 0;
			m_iWarningCount = 0;
			m_iErrorCount = 0;
			load (m_hTrace);
			load (m_hTraceWarning);
			load (m_hTraceError);
		}

		if (m_hTrace && ((CQtT120TraceFile *)m_hTrace)->m_bMaxReached)
		{
			load (m_hTrace);
		}

		if (m_hTraceWarning && dwMask < 20) 
		{
			T120_Write_Trace_Dev(m_hTraceWarning, m_lpszModule,
				m_pMapEntries[dwMask].m_lpszMaskDescription,
				lpsz);
			if (((CQtT120TraceFile *)m_hTraceWarning)->m_bMaxReached)
			{
				load (m_hTraceWarning);
			}
		}
		if (m_hTraceError && dwMask < 10)
		{
			T120_Write_Trace_Dev(m_hTraceError, m_lpszModule,
            m_pMapEntries[dwMask].m_lpszMaskDescription,
            lpsz);
			if (((CQtT120TraceFile *)m_hTraceError)->m_bMaxReached)
			{
				load (m_hTraceError);
			}
		}
		
#endif
    }
}

void CQtT120Trace::log_string(char* lpsz)
{
#ifdef QT_WIN32
	::OutputDebugStringA(lpsz);
	::OutputDebugStringA("\r\n");
	return;
#endif
	
/* Diable log file
if(m_hTrace_log)
T120_Write_Trace_Dev(m_hTrace_log, NULL, NULL, lpsz);
	*/
}
#endif

void Assert_Log(char* expression, char* file_name, int line_no)
{
	//    T120_LOG("Assertion (" << expression << ") failed at file " <<
	//        file_name << " line " << line_no);
}

void refresh_trace_config()
{
	(CQtT120Trace::instance())->read_config();
}




/////////////////////////////////////////////////////////////////////////////
// Base64 encode/decode
//
static char s_base64[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char s_base64_decode_table[256] = { 0 };

void xbase64_init_decode_table()
{
    int i;
    for(i = 0; i < 256; i++)
        s_base64_decode_table[i] = 0;
	
    unsigned char code = 0;
    for(i = (int)'A'; i <= (int)'Z'; i++)
    {
        s_base64_decode_table[i] = code;
        code++;
    }
	
    for(i = (int)'a'; i <= (int)'z'; i++)
    {
        s_base64_decode_table[i] = code;
        code++;
    }
	
    for(i = '0'; i <= '9'; i++)
    {
        s_base64_decode_table[i] = code;
        code++;
    }
	
    s_base64_decode_table[(int)'+'] = code++;
    s_base64_decode_table[(int)'/'] = code++;
	
    QT_ASSERTE(code == 64);
}

unsigned long xbase64_calc_encode_buf_size(unsigned char*, unsigned long data_len)
{
    return ((data_len / 3) + 1) * 4;
}

unsigned long xbase64_encode(unsigned char* data, unsigned long data_len, 
					  unsigned char* buf, unsigned long buf_len)
{
    QT_ASSERTE(data);
    QT_ASSERTE(buf_len >= xbase64_calc_encode_buf_size(data, data_len));
	
    unsigned char* cur_data = data;
    unsigned char* cur_buf = buf;
    unsigned long i;
    for(i = 0; i < data_len / 3; i++)
    {
        cur_buf[0] = (unsigned char)s_base64[(cur_data[0] & 0xFC) >> 2];
        cur_buf[1] = (unsigned char)s_base64[((cur_data[0] & 0x03) << 4) | ((cur_data[1] & 0xF0) >> 4)];
        cur_buf[2] = (unsigned char)s_base64[((cur_data[1] & 0x0F) << 2) | ((cur_data[2] & 0xC0) >> 6)];
        cur_buf[3] = (unsigned char)s_base64[cur_data[2] & 0x3F];
		
        cur_buf  += 4;
        cur_data += 3;
    }
	
    switch(data_len % 3)
    {
    case 0 :
        break;
		
    case 1 :
        cur_buf[0] = (unsigned char)s_base64[(cur_data[0] & 0xFC) >> 2];
        cur_buf[1] = (unsigned char)s_base64[(cur_data[0] & 0x03) << 4];
        cur_buf[2] = (unsigned char)'=';
        cur_buf[3] = (unsigned char)'=';
		
        cur_buf += 4;
        break;
		
    case 2 :
        cur_buf[0] = (unsigned char)s_base64[(cur_data[0] & 0xFC) >> 2];
        cur_buf[1] = (unsigned char)s_base64[((cur_data[0] & 0x03) << 4) | ((cur_data[1] & 0xF0) >> 4)];
        cur_buf[2] = (unsigned char)s_base64[(cur_data[1] & 0x0F) << 2];
        cur_buf[3] = (unsigned char)'=';
		
        cur_buf += 4;
        break;
    }
	
    return cur_buf - buf;
}

unsigned long xbase64_decode(unsigned char* buf, unsigned long buf_len, 
					  unsigned char* data, unsigned long)
{
    QT_ASSERTE(buf_len % 4 == 0);
	
    unsigned char* cur_buf = buf;
    unsigned char* cur_data = data;
	
    for(unsigned long i = 0; i < buf_len / 4; i++)
    {
        unsigned char bA, bB, bC, bD;
        bA = s_base64_decode_table[cur_buf[0]];
        bB = s_base64_decode_table[cur_buf[1]];
        bC = s_base64_decode_table[cur_buf[2]];
        bD = s_base64_decode_table[cur_buf[3]];
		
        cur_data[0] = (bA << 2) | (bB >> 4);
        cur_data[1] = ((bB & 0x0F) << 4) | (bC >> 2);
        cur_data[2] = ((bC & 0x03) << 6) | bD;
		
        if(cur_buf[2] != '=' && cur_buf[3] != '=')
        {
            cur_data += 3;
        }
        else
        {
            if(cur_buf[2] != '=')
            {
                cur_data[2] = 0;
                cur_data += 2;
            }
            else
            {
                cur_data[2] = 0;
                cur_data[1] = 0;
                cur_data += 1;
            }
        }
		
        cur_buf += 4;
    }
	
    return cur_data - data;
}

char* get_string_from_XML(char* src, char* tag, int max_len, char* buf, char* def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char tagx[256];
	sprintf(tagx, "<%s>", tag);
	
//	int index_begin = 0;
//	int index_end = 0;
	
	char* target = strstr(src, tagx);
	if(target == NULL) return def;
	target += strlen(tagx);
	while(target[0] != 0 && 
		(target[0] == ' ' || target[0] == '\r' || target[0] == '\n'
		|| target[0] == '\t')
		) target++;
	if(target[0] == 0) return def;
	
	sprintf(tagx, "</%s>", tag);
	char* target2 = strstr(target, tagx);
	if(target2 == NULL) return def;
	
	while(target2 - target > 0 &&
		(*(target2-1) == ' ' || *(target2-1) == '\r' || *(target2-1) == '\n'
		|| *(target2-1) == '\t')
		) target2--;
	int target_len = target2 - target;
	if(target_len > max_len) return def;
	
	strncpy(buf, target, target_len);
	buf[target_len] = 0;
	return buf;
}

unsigned long get_uint32_from_XML(char* src, char* tag, unsigned long def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[64];
	sprintf(sdef, "%lu", def);
	
	char buf[64];
	get_string_from_XML(src, tag, 64, buf, sdef);
	
	unsigned int ret = def;
	sscanf(buf, "%u", &ret);
	return ret;
}

int get_int32_from_XML(char* src, char* tag, unsigned long def)
{
	char sdef[64];
	sprintf(sdef, "%ld", def);
	
	char buf[64];
	get_string_from_XML(src, tag, 64, buf, sdef);
	
	int ret = def;
	sscanf(buf, "%d", &ret);
	return ret;
}

double get_double_from_XML(char* src, char* tag, double def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[256];
	sprintf(sdef, "%f", def);
	
	char buf[256];
	get_string_from_XML(src, tag, 256, buf, sdef);
	
	float ret = 0.0;
	int len = sscanf(buf, "%f", &ret);
	if(len == 0 || len == EOF) return def;
	
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// UNIX misc functions
//
unsigned long calculate_tick_interval(unsigned long start, unsigned long end)
{
    if(end >= start)
        return end - start;
	
    return (unsigned long)-1 - start + 1 + end;
}

#ifndef QT_WIN32
unsigned long get_tick_count()
{
	unsigned long   ret;
	struct  timeval time_val;
	
	gettimeofday(&time_val, NULL);
	ret = time_val.tv_sec * 1000 + time_val.tv_usec / 1000;
	
	return ret;
}
#endif //QT_WIN32

void ms_sleep(unsigned long milli_seconds)
{
#ifndef QT_WIN32
	struct timespec ts, rmts;
	
	ts.tv_sec = milli_seconds/1000;
	ts.tv_nsec = (milli_seconds%1000)*1000000;
	while(1)
	{
		int ret = nanosleep(&ts, &rmts);
		if(ret == 0) 
			break;
		if(ret == -1 && errno == EINTR)
		{
			ts = rmts;
		}
		else
		{
			QT_ASSERTE(FALSE);
		}
	}
#else //QT_WIN32
	
	Sleep(milli_seconds);
#endif //QT_WIN32
	
}

void output_debug_string(char*)
{
	//    T120_LOG(str);
}

#if !defined QT_MACOS || defined MachOSupport
void resolve_to_ip(char* lpszHostName, char* lpszIPAddress)
{
	struct hostent* he = gethostbyname((char*)lpszHostName);
	if(he)
	{
		char* lpszTemp = (char*)inet_ntoa((in_addr&)(*he->h_addr_list[0]));
		if(lpszTemp)
			strcpy(lpszIPAddress, lpszTemp);
	}
}

char* get_local_ip_address()
{
	char achBuf[256];
	achBuf[0] = 0;
	
	if(gethostname(achBuf, sizeof(achBuf)) == 0)
	{
		struct hostent* pHost = gethostbyname(achBuf);
		
		if(pHost)
		{
			in_addr in;
			
			memcpy(&in.s_addr, pHost->h_addr, pHost->h_length);
			char* addr = inet_ntoa(in);
			return addr;
		}
	}
	
	// return the local host predefined address
	return "127.0.0.1";
}
#endif

/////////////////////////////////////////////////////////////////////////////
// File based get_int_from_profile() and get_string_from_profile()
// implementation
//
char* trim_string(char* str)
{
	QT_ASSERTE(str);
	
	// trim tail
	char* cur_str = str + strlen(str) - 1;
	while(cur_str >= str)
	{
		if(strchr(" \t\r\n", *cur_str))
		{
			*cur_str = 0;
			cur_str--;
		}
		else
		{
			break;
		}
	}
	
	// trim head
	cur_str = str;
	while(*cur_str)
	{
		if(strchr(" \t\r\n", *cur_str))
			cur_str++;
		else
			break;
	}
	return cur_str;
}

unsigned char is_comment_line(char* cur_str)
{
	while(*cur_str)
	{
		if(strchr(" \t\r\n", *cur_str) != NULL)
			cur_str++;
		else
			return *cur_str == '#';
	}
	return FALSE;
}

unsigned char read_init_file(FILE* f, char* app_name, char* key_name,
					   char* ret_str, unsigned long len)
{
	char buf[2048];
	unsigned char find_section = FALSE;
	
	while(!feof(f))
	{
		buf[0] = 0;
		if(!fgets(buf, sizeof(buf), f))
			break;
		
		if(is_comment_line(buf))
			continue;
		
		if(strchr(buf, '[') && strchr(buf, ']'))
		{
			if(find_section)
			{
				// encounter another section
				return FALSE;
			}
			else
			{
				char* token = strchr(buf, '[');
				token++;
				char *p = strchr(buf, ']');
				if(p)
					*p = 0;
				//*strchr(buf, ']') = 0;
				
				token = trim_string(token);
#ifndef QT_WIN32
				if(strcasecmp(token, app_name) == 0)
					find_section = TRUE;
#else //QT_WIN32
				if(stricmp(token, app_name) == 0)
					find_section = TRUE;
				
#endif //QT_WIN32
			}
		}
		else
		{
			if(find_section)
			{
				char* token = strchr(buf, '=');
				if(token)
				{
					*token = 0;
					token++;
					
					char* pKey = trim_string(buf);
#ifndef QT_WIN32
					if(strcasecmp(pKey, key_name) == 0)
#else //QT_WIN32
						if(stricmp(pKey, key_name) == 0)
#endif //QT_WIN32
						{
							token = trim_string(token);
							if(*token == '\'' || *token == '"')
							{
								int str_len = strlen(token);
								if(token[str_len-1] == *token)
								{
									token[str_len-1] = 0;
									token++;
									token = trim_string(token);
								}
							}
							strncpy(ret_str, token, len);
							return TRUE;
						}
				}
			}
		}
	}
	return FALSE;
}

int get_int_from_profile(char* app_name, char* key_name,
						 int default_value, char* file_name)
{
	int i;
	
	FILE* f = fopen(file_name, "rt");
	if(!f)
		return default_value;
	
	//    SET_CLOSEONEXEC(fileno(f));
	
	char buf[256];
	if(read_init_file(f, app_name, key_name, buf, 256))
	{
		fclose(f);
		sscanf(buf, "%d", &i);
		return i;
	}
	fclose(f);
	
	return default_value;
}

unsigned long get_string_from_profile(char* app_name, char* key_name,
							   char* default_str, char* ret_str, unsigned long len, char* file_name)
{
	FILE* f = fopen(file_name, "rt");
	if(!f)
	{
		strncpy(ret_str, default_str, len - 1);
		return strlen(ret_str);
	}
	
	//    SET_CLOSEONEXEC(fileno(f));
	
	if(read_init_file(f, app_name, key_name, ret_str, len))
	{
		fclose(f);
		return strlen(ret_str);
	}
	fclose(f);
	
	strncpy(ret_str, default_str, len - 1);
	return strlen(ret_str);
}

// #ifndef QT_PORT_CLIENT //it is useless for client on cross platform when we use encryption trace
//#ifndef QT_PORT_CLIENT //it is useless for client on cross platform when we use encryption trace
char g_cur_env[512] = "QTEC_HOME_DIR";
// #else
// char g_cur_env[512] = "";
// #endif
//#else
//char g_cur_env[512] = "";
//#endif

CQtString g_qtec_config_file_name = "qtec.cfg";
#if !defined(QT_WIN32)
char g_def_qtec_home_dir[] = "/tmp/";
#else
const int G_DEF_qtec_HOME_DIR_MAX = 512;
char g_def_qtec_home_dir[G_DEF_qtec_HOME_DIR_MAX] = ".";
#endif //!defined(QT_WIN32)


void set_qtec_home_env(char* home_env)
{
	strncpy(g_cur_env, home_env, sizeof(g_cur_env));
}

void set_qtec_config_file_name(const CQtString &file)
{
	g_qtec_config_file_name = file;
}

char* get_qtec_home_dir()
{
	char* p = 0;
#if !defined(QT_WIN32)

// #ifndef QT_PORT_CLIENT //it is useless for client on cross platform when we use encryption trace
//#ifndef QT_PORT_CLIENT //it is useless for client on cross platform when we use encryption trace
	 p = getenv(g_cur_env); //it should has security about this API
// #else
// 	 p = NULL;
// #endif
//#else
//	 p = NULL;
//#endif

#else// !defined(QT_WIN32)
		GetCurrentDirectory(G_DEF_qtec_HOME_DIR_MAX,g_def_qtec_home_dir);
#endif //!defined(QT_WIN32)
	if (!p)
		p = g_def_qtec_home_dir;
	
	return p;
}

void get_config_file_name(char* file_name)
{
	char* qtec_home_dir = get_qtec_home_dir();
	if(qtec_home_dir)
	{
#if !defined(QT_WIN32)
		strcpy(file_name, qtec_home_dir);
		if(file_name[strlen(file_name) - 1] != '/')
			strcat(file_name, "/");

		CQtString strTmp(file_name);
		strTmp += "conf/";
		strTmp += g_qtec_config_file_name;
		memcpy(file_name, strTmp.c_str(), strTmp.length());
		file_name[strTmp.length()] = 0;

#else //!defined(QT_WIN32)
		strcpy(file_name, qtec_home_dir);
		if(file_name[strlen(file_name) - 1] != '\\')
			strcat(file_name, "\\");
		strcat(file_name, "conf\\qtec.cfg");
#endif // !defined(QT_WIN32)
	}
	else
	{
		CQtString strTmp("/tmp/");
		strTmp += g_qtec_config_file_name;
		memcpy(file_name, strTmp.c_str(), strTmp.length());
		file_name[strTmp.length()] = 0;
	}
}

void init_config(const char* cur_env)
{
	if(cur_env && strlen(cur_env) < sizeof(g_cur_env) - 1)
		strcpy(g_cur_env, cur_env);
}

unsigned char get_string_param(char* group, char* item_key, 
						 char* item_value, unsigned long len)
{
	char config_file_name[512];
	
	if(!item_value)
		return FALSE;
	
	get_config_file_name(config_file_name);
	item_value[0] = 0;
	
	get_string_from_profile(group, item_key,
		"", item_value, len, config_file_name);
	
	int str_len = strlen(item_value);
	if(str_len > 0)
	{
		// for old INI format
		if(item_value[str_len - 1] == ';')
			item_value[str_len - 1] = 0;
	}
	
	return str_len > 0;
}

int get_int_param(char* group, char* item_key)
{
	char str_value[256];
	
	if(get_string_param(group, item_key, str_value, 256))
	{
		int i;
		sscanf(str_value, "%d", &i);
		
		return i;
	}
	return 0;
}

unsigned short get_uint16_param(char* group, char* item_key)
{
	char str_value[256];
	
	if(get_string_param(group, item_key, str_value, 256))
	{
		int i;
		sscanf(str_value, "%d", &i);
		
		return (unsigned short)i;
	}
	
	return 0;
}

unsigned long get_uint32_param(char* group, char* item_key)
{
	char str_value[256];
	
	if(get_string_param(group, item_key, str_value, 256))
	{
		unsigned long i;
		sscanf(str_value, "%lu", &i);
		
		return i;
	}
	
	return 0;
}

unsigned char get_bool_param(char* group, char* item_key, 
					   unsigned char default_value)
{
	char str_value[256];
	
	if(get_string_param(group, item_key, str_value, 256))
#ifndef QT_WIN32
		return strcasecmp(str_value, "TRUE") == 0; 
#else //QT_WIN32
		return stricmp(str_value, "TRUE") == 0; 
#endif //QT_WIN32
	
	return default_value;
}

/*
* For compatibility with Solaris, provide get_exec_name(), which attempts to
* return the name of the current executable by looking it up in /proc.
*/
const char* get_exec_name(void)
{
#define QTDBUFLEN 1024 /* PATH_MAX */
	static char cmd_name_buf[QTDBUFLEN + 1]={0};
	static char *cmd_name = NULL;
	
#ifndef QT_WIN32
#ifdef MachOSupport
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	OSErr 	theErr;
	FSSpec 	theSpec;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	/* Find the application FSSpec */
	theErr = GetProcessInformation(&thePSN, &theInfo);
	memcpy(cmd_name_buf, theSpec.name+1,*theSpec.name);
	cmd_name = cmd_name_buf;
#else

	char pidfile[64];
	if (!cmd_name)
	{
		int bytes;
		int fd;
		
		sprintf(pidfile, "/proc/%d/cmdline", getpid());
		
		fd = open(pidfile, O_RDONLY, 0);
		bytes = read(fd, cmd_name_buf, 256);
		close(fd);
		
		cmd_name_buf[QTDBUFLEN] = '\0';
		cmd_name = cmd_name_buf;
	}
#endif
#else
	// budingc
	if (!cmd_name) {
		::GetModuleFileNameA(NULL, cmd_name_buf, sizeof(cmd_name_buf));
		cmd_name = cmd_name_buf;
	}

#endif // !QT_WIN32
	return(cmd_name);
}

const char* get_process_name()
{
	const char* pExecName = get_exec_name();
	
	if(pExecName)
	{
		const char* pch = pExecName + strlen(pExecName) - 1;
		while(pch > pExecName && *pch != '/')
			pch--;
		
		if(*pch == '/')
			pch++;
		
		return pch;
	}
	else
	{
		return "defproc";
	}
}



#define BETWEEN(c, x, y)	(c >= x && c <= y)
char* url_string_encode(char* src, char* dest, int max_len)
{
	QT_ASSERTE(src);
	QT_ASSERTE(dest);
	strcpy(dest, "");
	
	int len = strlen(src);
	int j = 0;
	for(int i=0; i<len && j<max_len; i++)
	{
		if(BETWEEN(src[i], 'a', 'z') || BETWEEN(src[i], 'A', 'Z') 
			|| BETWEEN(src[i], '0', '9')) 
			dest[j++] = src[i];
		else if (src[i] == ' ')
			dest[j++] = '+';
		else 
		{
			char szTmp[8];
			dest[j++] = '%';
			sprintf(szTmp, "%x", (unsigned char)src[i]);
			dest[j++] = szTmp[0];
			dest[j++] = szTmp[1];
		}
	}
	
	dest[j] = 0;
	
	return dest;
}

unsigned long xml_get_uint32(char* src, char* tag, unsigned long def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[64];
	sprintf(sdef, "%lu", def);
	
	char buf[64];
	xml_get_string(src, tag, 64, buf, sdef);
	
	unsigned long ret = def;
	sscanf(buf, "%lu", &ret);
	return ret;
}

unsigned long xml_get_int32(char* src, char* tag, unsigned long def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[64];
	sprintf(sdef, "%ld", def);
	
	char buf[64];
	xml_get_string(src, tag, 64, buf, sdef);
	
	unsigned long ret = def;
	sscanf(buf, "%ld", &ret);
	return ret;
}

unsigned short xml_get_uint16(char* src, char* tag, unsigned short def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[64];
	sprintf(sdef, "%d", def);
	
	char buf[64];
	xml_get_string(src, tag, 64, buf, sdef);
	
	unsigned long ret = def;
	sscanf(buf, "%ld", &ret);
	return (unsigned short)ret;
}

unsigned short xml_get_int16(char* src, char* tag, unsigned short def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	
	char sdef[64];
	sprintf(sdef, "%d", def);
	
	char buf[64];
	xml_get_string(src, tag, 64, buf, sdef);
	
	unsigned long ret = def;
	sscanf(buf, "%ld", &ret);
	return (unsigned short)ret;
}

char* xml_get_string(char* src, char* tag, unsigned short max_len, 
								char* dest, char* def)
{
	QT_ASSERTE(src);
	QT_ASSERTE(tag);
	QT_ASSERTE(dest);
	
	char tagx[256];
	sprintf(tagx, "<%s>", tag);
	
//	int index_begin = 0;
//	int index_end = 0;
	
	char* target = strstr(src, tagx);
	if(target == NULL) return def;
	target += strlen(tagx);
	while(target[0] != 0 && 
		(target[0] == ' ' || target[0] == '\r' || target[0] == '\n'
		|| target[0] == '\t')
		) target++;
	if(target[0] == 0) return def;
	
	sprintf(tagx, "</%s>", tag);
	char* target2 = strstr(target, tagx);
	if(target2 == NULL) return def;
	
	while(target2 - target > 0 &&
		(*(target2-1) == ' ' || *(target2-1) == '\r' || *(target2-1) == '\n'
		|| *(target2-1) == '\t')
		) target2--;
	
	int target_len = target2 - target;
	if(target_len > max_len) return def;
	
	strncpy(dest, target, target_len);
	dest[target_len] = 0;
	
	return dest;
}

unsigned char transport_address_parse(
										   char* transport_address, char* protocol_type, int max_protocol_len, 
										   char* host_ip, int max_host_ip_len, unsigned short* port)
{
	QT_ASSERTE(transport_address);
	QT_ASSERTE(protocol_type);
	QT_ASSERTE(host_ip);
	QT_ASSERTE(port);
	
	char tmp_protocol[64] = "";
	char tmp_host_name[64] = "";
	char tmp_host_ip[64] = "";
	char tmp_port[64] = "";
	
	if(strlen(transport_address)>64)
	{
		return FALSE;
	}
	
	char* tmp = strstr(transport_address, "://");
	if(!tmp)
	{
		return FALSE;
	}
	strncpy(tmp_protocol, transport_address, tmp-transport_address);
	tmp_protocol[tmp-transport_address] = 0;
	if((int)strlen(tmp_protocol)<max_protocol_len)
		strcpy(protocol_type, tmp_protocol);
	else
	{
		strncpy(protocol_type, tmp_protocol, max_protocol_len-1);
		protocol_type[max_protocol_len-1] = 0;
	}
	
	char* tmp1 = strstr(tmp+3, ":");
	if(!tmp1)
	{
		return FALSE;
	}
	strncpy(tmp_host_name, tmp+3, tmp1-tmp-3);
	tmp_host_name[tmp1-tmp-3] = 0;
	
	if(!is_ip_address(tmp_host_name))
	{
		resolve_2_ip(tmp_host_name, tmp_host_ip);
		if(strlen(tmp_host_ip) == 0)
			return FALSE;
	}
	else
		strcpy(tmp_host_ip, tmp_host_name);
	
	
	if((int)strlen(tmp_host_ip)<max_host_ip_len)
		strcpy(host_ip, tmp_host_ip);
	else
	{
		strncpy(host_ip, tmp_host_ip, max_host_ip_len-1);
		host_ip[max_host_ip_len-1] = 0;
	}
	
	tmp1++;
	if(!(*tmp1))
	{
		QT_ASSERTE(FALSE);
		return FALSE;
	}
	strcpy(tmp_port, tmp1);
	int port_1;
	sscanf(tmp_port, "%d", &port_1);
	*port = port_1;
	
	return TRUE;
}

unsigned char is_ip_address(char* lpsz)
{
	if(!lpsz)
	{
		QT_ERROR_TRACE("is_ip_address: ip string is null");
		return FALSE;
	}
	
    char* lpszTemp = lpsz;
    while(*lpszTemp)
    {
        if(*lpszTemp != ' ' &&  *lpszTemp != '\t')
        {
            if(*lpszTemp != '.' && !(*lpszTemp >= '0' && *lpszTemp <= '9'))
                return FALSE;
        }
        lpszTemp++;
    }
    return TRUE;
}
#if !defined QT_MACOS || defined MachOSupport
void resolve_2_ip(char* host_name, char* ip_address)
{
	QT_ASSERTE(host_name != NULL);
	QT_ASSERTE(ip_address != NULL);
	if(!host_name || !ip_address)
	{
		QT_ERROR_TRACE("resolve_2_ip: No host_name: " << host_name 
			<< "; or ip_address: " << ip_address << "; given");
		return;
	}
	
    struct hostent* he = gethostbyname(host_name);
    if(he)
    {
        char* tmp = (char*)inet_ntoa((in_addr&)(*he->h_addr_list[0]));
        if(tmp)
            strcpy(ip_address, tmp);
    }
	else
	{
		QT_ERROR_TRACE("resolve_2_ip: can't resolve host_name: " << host_name);
	}
}

unsigned long ip_2_id(char* ip_address)
{
	unsigned long id;
	char digit_ip[128] = "";
	if(is_ip_address(ip_address))
	{
		strcpy(digit_ip, ip_address);
	}
	else
	{
		resolve_2_ip(ip_address, digit_ip);
	}
	
	if(strlen(digit_ip) == 0)
		return 0;
	
	int tmp_ip[4];
	sscanf(digit_ip, "%d.%d.%d.%d", &tmp_ip[0], &tmp_ip[1], &tmp_ip[2], &tmp_ip[3]);
	id = tmp_ip[0];
	id <<= 8;
	id += tmp_ip[1];
	id <<= 8;
	id += tmp_ip[2];
	id <<= 8;
	id += tmp_ip[3];
	
	return id;
}

char* id_2_ip(unsigned long ip_address)
{
	static char ip_str[64];
	
	unsigned int tmp_ip[4];
	tmp_ip[3] = ip_address & 0xff;
	tmp_ip[2] = (ip_address >> 8) & 0xff;
	tmp_ip[1] = (ip_address >> 16) & 0xff;
	tmp_ip[0] = (ip_address >> 24) & 0xff;
	
	sprintf(ip_str, "%d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
	
	return ip_str;
}
#endif


#ifdef QT_WIN32
char g_local_ip[64] = "";
char* get_local_ip()
{
	if(strlen(g_local_ip) != 0) return g_local_ip;
	
	char buffer[256];
	WSADATA data;
	WORD version = 0x0101;
	int result = WSAStartup(version, &data);
	if(gethostname(buffer, sizeof(buffer)) == 0) 
	{
		HOSTENT* host = gethostbyname(buffer);
		QT_ASSERTE_RETURN(host, NULL);
		in_addr in;
		memcpy(&in.S_un.S_addr, host->h_addr, host->h_length);
		char* addr = inet_ntoa(in);
		if(addr)
			strcpy(g_local_ip, addr);
		else
			strcpy(g_local_ip, "0.0.0.0");
		
		WSACleanup();
	}
	else
	{
		WSACleanup();
		strcpy(g_local_ip, "0.0.0.0");
	}
	
	return g_local_ip;
}
#else
#if !defined QT_MACOS || defined MachOSupport
char g_local_ip[64] = "";
char* get_local_ip()
{
	if(strlen(g_local_ip) != 0) return g_local_ip;
	
	strcpy(g_local_ip, "0.0.0.0");
	char buffer[256];
	if(gethostname(buffer, sizeof(buffer)) == 0) 
	{
		struct hostent* host = gethostbyname(buffer);
		QT_ASSERTE(host);
		in_addr in;
		memcpy(&in.s_addr, host->h_addr, host->h_length);
		char* addr = inet_ntoa(in);
		if(addr)
			strcpy(g_local_ip, addr);
	}
	
	return g_local_ip;
}
#endif
#endif	// QT_WIN32
