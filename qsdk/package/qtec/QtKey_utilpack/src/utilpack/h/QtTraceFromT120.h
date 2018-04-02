/*------------------------------------------------------*/
/* Trace functions                                      */
/*                                                      */
/* QtTraceFromT120.h                                    */
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

/*  
	Trace utility changes for Eureka platform

	1. Trace file can be set to enable or disable a certain type of trace. 
		e.g. disable warning, pdu or function
		The format in qtec.cfg is:

		[Trace]       
		XXXX.Error=TRUE;
		XXXX.Warning=FALSE;
		XXXX.Info=FALSE;

		XXXX is the process name to handle. 
		The default value is as in this list: 
			{ 0, "Error",      TRUE  },

			{ 1, "Server",      TRUE  },
			{ 10, "Warning",    TRUE  },

			{ 11, "Meeting",    TRUE  },

			{ 12, "Session",    TRUE  },

			{ 13, "User",    TRUE  },

			{ 14, "Statistic",    TRUE  },
			{ 20, "Info",	   TRUE  },
			{ 21, "State",      TRUE  },
			{ 22, "PDU",        FALSE },
			{ 23, "Func",       FALSE },
			{ 24, "Tick",       FALSE },
			{ 25, "Detail",     FALSE }



		This setting can be changed at runtime.

	2. Trace file can be set to flush each time a trace is written, or 
		bufferred before written to file in group to reduce overhead. 
		This can be changed at runtime.

		The format in qtec.cfg is:

		[Trace]       
		FlushPerLines=1

		or 

		FlushPerLines=200

		Default is 100. This is to reduce the overhead brought by IO operation.

	3. Only 1 file per process. file size can be set in qtec.cfg. the 
		default size is 20MB. The file name format is process_name+date.process_id.

		The format in qtec.cfg is:

		[Trace]       
		XXXX=100MB

		XXXX is the process name.

	
	4. To allow prcoess realtime refresh trace config through signal, provide 
		an API as follow:
		
		void refersh_trace_config();

		A signal handler should call this function to inform trace module 
		to refresh its config by reading qtec.cfg

*/


#ifndef QTTRACEFROMT120_H
#define QTTRACEFROMT120_H

#ifndef QT_BIND_WITH_EUREKA
#include "QtDefines.h"
#include "QtMutex.h"
#include "QtStdCpp.h"

#if defined QT_WIN32 || defined QT_PORT_CLIENT || defined QT_MACOS
#define QT_QTEC_UNIFIED_TRACE
typedef int ( * QT_QTEC_TRACE)( const char* format, ...);

#if !defined QT_HMODULE
#define QT_HMODULE		void *
#endif

#ifndef QT_WIN32
	#ifndef QT_MACOS
		#include <dlfcn.h>
	#else
		#ifdef MachOSupport
			#include <dlfcn.h>
		#else
			#define RTLD_LAZY			0x1
			#define RTLD_NOW			0x2
			#define RTLD_LOCAL			0x4
			#define RTLD_GLOBAL			0x8
			#define RTLD_NOLOAD			0x10
			#define RTLD_NODELETE		0x80
		#endif	//MachOSupport	
	#endif

#if !defined QT_MACOS
#define LoadLibrary		dlopen
#define GetProcAddress	dlsym
#define FreeLibrary		dlclose
#endif

typedef int ( * QT_QTEC_TRACE2)( const TCHAR* dllname, const TCHAR* format, ...);
#endif

#endif
/////////////////////////////////////////////////////////////////////////////
//
// Macro used in trace module
//

//#define EXPORTMODIFIER

#define T120TRACE_MAX_PROCESS				10
#define T120TRACE_MAX_PROCESS_NAME			32
#define T120TRACE_MAX_MODULE				20
#define T120TRACE_MAX_MASK					32
#define T120TRACE_MAX_TRACE_LEN				1024

#define T120TRACE_MAX_MODULE_NAME			32
#define T120TRACE_MAX_MASK_NAME				32

//#define T120TRACE_DEFAULT_FLUSH_FREQ		100
#define T120TRACE_DEFAULT_FLUSH_FREQ		1
#ifdef QT_WIN32
  #define T120TRACE_DEFAULT_TRACE_SIZE		1*1024*1024L
#else
  #define T120TRACE_DEFAULT_TRACE_SIZE		20*1024*1024L
#endif // QT_WIN32

#ifndef QT_WIN32
typedef long long int	_int64;
#endif
/////////////////////////////////////////////////////////////////////////////
// CQtT120TraceFile class
//
class QT_OS_EXPORT CQtT120TraceFile
{
	CQtT120TraceFile(const CQtT120TraceFile &);
	CQtT120TraceFile & operator=(const CQtT120TraceFile &);
public :
    CQtT120TraceFile(const char* pszFileName, unsigned long lMaxSize = 0, unsigned char bShared = FALSE, unsigned char bEnabled = TRUE);
    ~CQtT120TraceFile();

    unsigned char is_same_file(const char* pszFileName);
	unsigned char refresh_settings(unsigned long lMaxSize, int nFreq);

	void flush_buffer();

	void write(const char* lpszModule, 
        const char* lpszDescription, 
        const char* lpsz);
	void write_shared(const char* lpszModule, 
        const char* lpszDescription, 
        const char* lpsz);
//	boolean safe_lock();

//	void safe_unlock();

public :
    CQtT120TraceFile* m_pNext;
	unsigned char m_bMaxReached;

private :
//    CQtMutexThreadRecursive m_mutex;
//	unsigned char m_busy;

    FILE* m_pFile;
    char* m_pFileName;
    unsigned long m_lMaxSize;
    int m_nFlushFreq;
    unsigned char m_bShared;

    int m_nCurrentFileId;
    long m_nCurrentTraceLines;
#if (!defined (QT_WIN32) && !defined(QT_MACOS)) || defined MachOSupport 
    struct flock m_lock;
    struct flock m_unlock;
#endif

	char* m_buffer;
    unsigned long m_pos;
	int	m_current_line;
	int m_nMagicNumber;
	int m_nFileHandle;
};

/////////////////////////////////////////////////////////////////////////////
// CQtT120TraceFileMgr class
//
class QT_OS_EXPORT CQtT120TraceFileMgr
{
public :
    CQtT120TraceFileMgr();
    ~CQtT120TraceFileMgr();

public :
    void* open(const char* pszFileName, unsigned long lMaxSize = 0, unsigned char bShared = FALSE, unsigned char bEnabled = TRUE);
	void close(void* hTrace);
    void write(void* hHandle, const char* lpszModule, 
        const char* lpszDescription, 
        const char* lpsz);

	void refresh_settings(unsigned long lMaxSize, int nFreq);
    void init_reg_filter();

protected :
    CQtT120TraceFile* m_pHead;
    int m_cCount;

    CQtMutexThreadRecursive m_mutex;
};

/////////////////////////////////////////////////////////////////////////////
// CQtT120TraceMapEntry for macro support
//
//
//  This trace utility supports following trace syntax
//
//  // module trace declaration
//  DECLARE_TRACE_MAP(module)
//  T120TRACE(module, mask, str)
//
//  // module trace definition
//  BEGIN_TRACE_MAP(module)
//      TRACE_MASK(mask, description, enable)
//      TRACE_MASK(mask, description, enable)
//  END_TRACE_MAP(module)
//

struct QT_OS_EXPORT CQtT120TraceMapEntry
{
    unsigned long   m_dwMask;
    char*  m_lpszMaskDescription;
    unsigned char    m_bEnable;
};

/////////////////////////////////////////////////////////////////////////////
// CQtT120Trace
// This class is used for supporting the trace macros
//
class QT_OS_EXPORT CQtT120Trace
{
public :
    CQtT120Trace(char* lpszModule, CQtT120TraceMapEntry mapEntries[],
        char* lpszPrivateInfo = NULL, unsigned char bShared = FALSE);

    virtual ~CQtT120Trace();

    unsigned char load(void * hTrace = NULL);
	void read_config();

    const char* get_trace_dir();
    const char* get_process_name();

public :
    typedef enum Ordix
    {
        hex     = 0,
        decimal = 1
    } Ordix_enum;

    // text-based formatting class
    class QT_OS_EXPORT Text_Formator
    {
    public :
        Text_Formator(char* lpszBuf, unsigned long dwBufSize);
        virtual ~Text_Formator();

    public :
        void reset();
        Text_Formator& operator << (char ch);
        Text_Formator& operator << (unsigned char ch);
        Text_Formator& operator << (short s);
        Text_Formator& operator << (unsigned short s);
        Text_Formator& operator << (int i);
        Text_Formator& operator << (unsigned int i);
        Text_Formator& operator << (long l);
        Text_Formator& operator << (unsigned long l);
        Text_Formator& operator << (float f);
        Text_Formator& operator << (double d);
        Text_Formator& operator << (const char* lpsz);
        Text_Formator& operator << (void* lpv);
        Text_Formator& operator << (Ordix ordix);
		Text_Formator& operator << (const CQtString& str);
		Text_Formator& operator << (const _int64 ll);
        operator char*();

    private :
        void set_hex_flag(unsigned char bValue);
        unsigned char get_hex_flag();
        void advance(const char* lpsz);

    private :
        char*  m_lpszBuf;
        unsigned long  m_dwSize;
        unsigned long  m_dwPos;

        unsigned char m_bHex;
    };

public :
    // API methods
    void trace_string(unsigned long dwMask, char* lpsz);
    void log_string(char* lpsz);
	static CQtT120Trace* instance();

private :
	static CQtT120Trace* t120_trace;
    char* m_lpszModule;
	unsigned long m_nMapEntries;
    CQtT120TraceMapEntry* m_pMapEntries;

    char* m_lpszPrivateInfo;
    unsigned char m_bShared;
    unsigned char m_bEnabled;

    void* m_hTrace;

	void* m_hTraceWarning;

	void* m_hTraceError;
    void* m_hTrace_log;

	unsigned char m_bMultiLogs;

	int m_iCurrCount;

	int m_iWarningCount;

	int m_iErrorCount;

	int m_iCurrInfoDay;

	int m_iCurrInfoMonth;

	int m_processid;
	
	CQtMutexThreadRecursive m_mutex;
#if defined QT_QTEC_UNIFIED_TRACE
	QT_HMODULE m_hTraceHandle;
public:
	void Close(); //for bug303444, need unload wbxtrace.dll when unload MMP
private:
#if defined QT_WIN32
	QT_QTEC_TRACE m_fpDebugTrace;
	QT_QTEC_TRACE m_fpInfoTrace;
	QT_QTEC_TRACE m_fpWarnTrace;
	QT_QTEC_TRACE m_fpErrorTrace;
#else	
	QT_QTEC_TRACE2 m_fpDebugTrace;
	QT_QTEC_TRACE2 m_fpInfoTrace;
	QT_QTEC_TRACE2 m_fpWarnTrace;
	QT_QTEC_TRACE2 m_fpErrorTrace;
#endif
#endif
};

/////////////////////////////////////////////////////////////////////////////
// Trace macros
//
// This is for use by the following MACROs.
#if ((defined (QT_WIN32) && defined (QT_MMP) && !defined QT_OUTPUT_TO_FILE && !defined QT_QTEC_UNIFIED_TRACE))

#include "QtQTECTrace.h"
#define QTECTRC_MMP_VERSION	1
typedef struct {
	WORD	prefix;
	BYTE	major_version;
	BYTE	minor_version;
} WbxMMPTraceHeader;

QTECTRC_DECLARE_MODULE(MMP);

#ifdef QT_BOTH_OUTPUT
#define WRITE_TRACE(level, event, data)	\
do{\
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator tformator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	tformator << data;\
	QTECTRC_WRITE(MMP, level, event, tformator);	\
	DWORD mask = 0;\
	switch(level)	{\
	case 0: mask = 0; break;\
	case 1: mask = 10; break;\
	case 2: mask = 20; break;\
	case 3: mask = 21; break;\
	default:\
		mask = level + 20; break;\
	}\
	(CQtT120Trace::instance())->trace_string(mask, tformator); \
} while(0);
#else
#define WRITE_TRACE(level, event, data)	\
	do{\
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator tformator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	tformator << data;\
	QTECTRC_WRITE(MMP, level, event, tformator);	\
	} while(0);

#endif

#ifdef QT_BOTH_OUTPUT
#define WRITE_TRACE_LOG(level, event, data) \
do{\
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator tformator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	tformator << data;\
	QTECTRC_WRITE_LOG(MMP, level, event, tformator); \
	ARCHIVE_TRACE(level, event, data); \
	DWORD mask = 0;\
	switch(level)	{\
	case 0: mask = 0; break;\
	case 1: mask = 10; break;\
	case 2: mask = 20; break;\
	case 3: mask = 21; break;\
	default:\
		mask = level + 20; break;\
	}\
	(CQtT120Trace::instance())->log_string(mask, tformator); \
}while(0);
#else
#define WRITE_TRACE_LOG(level, event, data) \
	do{\
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator tformator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	tformator << data;\
	QTECTRC_WRITE_LOG(MMP, level, event, tformator); \
	ARCHIVE_TRACE(level, event, data); \
	}while(0);
#endif
#define QT_T120TRACE(level, str)  {             \
	WRITE_TRACE(level, 0, str);					\
}

#define T120_LOG(str) {                         \
	WRITE_TRACE(2, 0, str);					\
}

#define QT_T120PTT_TRACE(str)	{				\
	WRITE_TRACE(2, 0, str);					\
}
class QT_OS_EXPORT CQtT120FuncTracer
{
public :
	CQtT120FuncTracer(const char* str)
	{
		buffer = str;
		QT_T120TRACE(5, "Enter " << ((char *) buffer));
	}
	
	virtual ~CQtT120FuncTracer()
	{
		QT_T120TRACE(5, "Leave " << ((char *) buffer));
	}
	
private:
    const char* buffer;
};

#define ERRTRACE(str) QT_T120TRACE(0, str)
#define WARNINGTRACE(str) QT_T120TRACE(1, str)
#define INFOTRACE(str) QT_T120TRACE(2, str)
#define STATETRACE(str) QT_T120TRACE(3, str)
#define PDUTRACE(str) QT_T120TRACE(4, str)
#define FUNCTRACE(str) CQtT120FuncTracer theFUNCTRACE(str);
#define TICKTRACE(str) QT_T120TRACE(6, str)
#define DETAILTRACE(str) QT_T120TRACE(7, str)

#define ERRTRACE_THIS(str)		ERRTRACE(str << " this=" << this)
#define WARNINGTRACE_THIS(str)	WARNINGTRACE(str << " this=" << this)
#define INFOTRACE_THIS(str)		INFOTRACE(str << " this=" << this)
#define STATETRACE_THIS(str)	STATETRACE(str << " this=" << this)
#define PDUTRACE_THIS(str)		PDUTRACE(str << " this=" << this)
#define TICKTRACE_THIS(str)		TICKTRACE(str << " this=" << this)
#define DETAILTRACE_THIS(str)	DETAILTRACE(str << " this=" << this)

#else ////(defined (QT_WIN32) && defined (QT_MMP) && !defined QT_OUTPUT_TO_FILE && !defined QT_QTEC_UNIFIED_TRACE)
#define QT_T120TRACE(mask, str)                   \
    {                                               \
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator formator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	(CQtT120Trace::instance())->trace_string(mask, formator << str); \
}

#define T120_LOG(str)                                 \
    {                                               \
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];   \
	CQtT120Trace::Text_Formator formator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	(CQtT120Trace::instance())->log_string(formator << str);        \
}

extern CQtT120Trace g_ptt_log_object;
#define QT_T120PTT_TRACE(str)						\
{                                               \
	char achFormatBuf[T120TRACE_MAX_TRACE_LEN];    \
	CQtT120Trace::Text_Formator formator(achFormatBuf, T120TRACE_MAX_TRACE_LEN); \
	g_ptt_log_object.trace_string(mask, formator << str); \
}
class QT_OS_EXPORT CQtT120FuncTracer
{
public :
	CQtT120FuncTracer(const char* str)
	{
		buffer = str;
		QT_T120TRACE(23, "Enter " << ((char *) buffer));
	}
	
	virtual ~CQtT120FuncTracer()
	{
		QT_T120TRACE(23, "Leave " << ((char *) buffer));
	}
	
private:
    const char* buffer;
};

#if !defined QT_MACOS || defined MachOSupport

#define ERRTRACE(str) QT_T120TRACE(0, str)
#define WARNINGTRACE(str) QT_T120TRACE(10, str)
#define INFOTRACE(str) QT_T120TRACE(20, str)
#define STATETRACE(str) QT_T120TRACE(21, str)
#define PDUTRACE(str) QT_T120TRACE(22, str)
#define FUNCTRACE(str) CQtT120FuncTracer theFUNCTRACE(str);
#define TICKTRACE(str) QT_T120TRACE(24, str)
#define DETAILTRACE(str) QT_T120TRACE(25, str)

#define ERRTRACE_THIS(str)		ERRTRACE(str << " this=" << this)
#define WARNINGTRACE_THIS(str)	WARNINGTRACE(str << " this=" << this)
#define INFOTRACE_THIS(str)		INFOTRACE(str << " this=" << this)
#define STATETRACE_THIS(str)	STATETRACE(str << " this=" << this)
#define PDUTRACE_THIS(str)		PDUTRACE(str << " this=" << this)
#define TICKTRACE_THIS(str)		TICKTRACE(str << " this=" << this)
#define DETAILTRACE_THIS(str)	DETAILTRACE(str << " this=" << this)
#endif // !QT_MACOS
#endif
/////////////////////////////////////////////////////////////////////////////
//
// Public utility function for refreshing trace configuration 
//

//void QT_OS_EXPORT refresh_trace_config();
#ifdef QT_SUPPORT_T120_UTIL
/////////////////////////////////////////////////////////////////////////////
//
// Note: Only Audio+ will define QT_SUPPORT_T120_UTIL!!!
// Please do not use these obsolete macros and functions in other projects!
//
/////////////////////////////////////////////////////////////////////////////
//utility functions
//

QT_OS_EXPORT void xbase64_init_decode_table();
QT_OS_EXPORT unsigned long xbase64_calc_encode_buf_size(unsigned char* data, unsigned long len);
QT_OS_EXPORT unsigned long xbase64_encode(unsigned char* data, unsigned long data_len, 
							 unsigned char* buf, unsigned long buf_len);
QT_OS_EXPORT unsigned long xbase64_decode(unsigned char* buf, unsigned long buf_len, 
							 unsigned char* data, unsigned long data_len);
QT_OS_EXPORT unsigned long calculate_tick_interval(unsigned long start, unsigned long end);

#ifdef QT_WIN32
#define get_tick_count GetTickCount
#define output_debug_string OutputDebugString
#else
QT_OS_EXPORT unsigned long get_tick_count();
QT_OS_EXPORT void output_debug_string(char* str);
#endif

QT_OS_EXPORT void init_config(const char* cur_env);
QT_OS_EXPORT unsigned char get_string_param(char* group, char* item_key, char* item_value, unsigned long len);
QT_OS_EXPORT int get_int_param(char* group, char* item_key);
QT_OS_EXPORT unsigned short get_uint16_param(char* group, char* item_key);
QT_OS_EXPORT unsigned long get_uint32_param(char* group, char* item_key);
QT_OS_EXPORT unsigned char get_bool_param(char* group, char* item_key, unsigned char default_value);
QT_OS_EXPORT void set_qtec_home_env(char* home_env);
QT_OS_EXPORT char* get_qtec_home_dir();
QT_OS_EXPORT const char* get_process_name();
QT_OS_EXPORT const char* get_exec_name(void);

QT_OS_EXPORT unsigned char transport_address_parse(char* transport_address, 
								char* protocol_type, int max_protocol_len, 
								char* host_ip, int max_host_ip_len, unsigned short* port);
QT_OS_EXPORT char* url_string_encode(char* src, char* dest, int max_len);
QT_OS_EXPORT unsigned long xml_get_uint32(char* src, char* tag, unsigned long def);
QT_OS_EXPORT unsigned long xml_get_int32(char* src, char* tag, unsigned long def);
QT_OS_EXPORT unsigned short xml_get_uint16(char* src, char* tag, unsigned short def);
QT_OS_EXPORT unsigned short xml_get_int16(char* src, char* tag, unsigned short def);
QT_OS_EXPORT char* xml_get_string(char* src, char* tag, unsigned short max_length, 
					 char* dest, char* def);
QT_OS_EXPORT char* get_local_ip();
QT_OS_EXPORT char* get_local_ip_address();
QT_OS_EXPORT unsigned char is_ip_address(char* sz);
QT_OS_EXPORT void resolve_2_ip(char* host_name, char* ip_address);
QT_OS_EXPORT unsigned long ip_2_id(char* ip_address);
QT_OS_EXPORT char* id_2_ip(unsigned long ip_address);
QT_OS_EXPORT void ms_sleep(unsigned long milli_seconds);

#endif // QT_SUPPORT_T120_UTIL

#else//QT_BIND_WITH_EUREKA

extern "C"
{
	extern void init_config(const char* cur_env);
	extern unsigned char get_string_param(char* group, char* item_key, char* item_value, unsigned long len);
	extern int get_int_param(char* group, char* item_key);
	extern unsigned short get_uint16_param(char* group, char* item_key);
	extern unsigned long get_uint32_param(char* group, char* item_key);
	extern unsigned char get_bool_param(char* group, char* item_key, unsigned char default_value);
	extern void set_qtec_home_env(char* home_env);
	extern void set_qtec_config_file_name(const CQtString &file);
	extern char* get_qtec_home_dir();
	extern const char* get_process_name();
	extern const char* get_exec_name(void);
	extern unsigned long get_tick_count();
}

#define QT_T120TRACE(mask, str)                   
#define T120_LOG(str)                             
#endif  // !QTTRACEFROMT120_H
#endif

