/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Client Trace C++ Interface                                              */
/*                                                                         */
/* Author:                                                                 */
/*      zhubin (zhubin@qtec.cn)                                            */
/*                                                                         */
/* History:                                                                */
/*      2017/02/15   Created                                               */
/*-------------------------------------------------------------------------*/

#ifndef __QTECTRACE_H_
#define __QTECTRACE_H_

#include "QtDefines.h"
#include "QtStdCpp.h"
#define	QTECTRC_MAX_ALLOC_FILE_SIZE	(320 * 1024)

#define	QTECTRC_MODULE_MAX_NUM	4
#define	QTECTRC_MODULE_TYPE_TP	0
#define	QTECTRC_MODULE_TYPE_ARM	1
#define	QTECTRC_MODULE_TYPE_MMP	2

#define	QTECTRC_FORMAT_TYPE_NUM		1
#define	QTECTRC_FORMAT_TYPE_STRING	2
#define	QTECTRC_FORMAT_TYPE_BINARY	3

#define QTECTRC_OS_TYPE_WIN		0
#define QTECTRC_OS_TYPE_WINCE	1
#define QTECTRC_OS_TYPE_SOLARIS	2
#define QTECTRC_OS_TYPE_LINUX	3
#define QTECTRC_OS_TYPE_HPUX		4
#define QTECTRC_OS_TYPE_MAC		5

typedef struct {
	DWORD	pid;
} WbxTraceMgrItem;

#define	QTECTRC_STACK_ITEM_MAX_LENGTH	64
#define	QTECTRC_STACK_ITEM_MAX_COUNT		64
#define	QTECTRC_STACK_THREAD_MAX_COUNT	4
typedef struct {
	DWORD		thread_id;
	BYTE		stack_depth;
	BYTE		dummy;
	WORD		stack_additional;
	char		stack[QTECTRC_STACK_ITEM_MAX_COUNT][QTECTRC_STACK_ITEM_MAX_LENGTH];
} WbxTraceFileThread;

typedef struct {
	BYTE		status;
	BYTE		version;
	WORD		dummy;
	DWORD		start_offset;
	DWORD		static_size;
	DWORD		log_size;
	DWORD		trace_size;
} WbxTraceFileModule;

typedef struct {
	BYTE		header_size;
	BYTE		os_type;
	BYTE		module_num;
	BYTE		thread_num;
	DWORD		pid;
	DWORD		alloc_size;
	DWORD		used_size;
	DWORD		start_sec;
	DWORD		start_msec;
	DWORD		start_tick;
	DWORD		last_modified_tick;
	WbxTraceFileModule	module_table[QTECTRC_MODULE_MAX_NUM];
} WbxTraceFileHeader;

typedef struct {
	WORD		size;
	BYTE		level;
	BYTE		event;
	DWORD		tick;
	WORD		thread_id;
	WORD		format;
} WbxTraceBlock;

#define QTECTRC_MGR_LIST_SIZE	100

class QT_OS_EXPORT CWbxTraceFormator
{
public :
    CWbxTraceFormator()
    {
		m_count = 0;
		m_format = 0;
		m_size = 0;
		m_lpTempBinary = NULL;
    }

    virtual ~CWbxTraceFormator()
    {
    }

public:
    CWbxTraceFormator& operator << (DWORD data)
    {
		if (m_lpTempBinary)
		{
			Add(QTECTRC_FORMAT_TYPE_BINARY, 2, m_lpTempBinary, data);
			m_lpTempBinary = NULL;
		}
		else
			Add(QTECTRC_FORMAT_TYPE_NUM, 0, (LPVOID)data, 4);
		return *this;
    }

	CWbxTraceFormator& operator << (int data)
	{
		return *this << (DWORD)data;
	}
	CWbxTraceFormator& operator << (unsigned int data)
	{
		return *this << (DWORD)data;
	}
	
	CWbxTraceFormator& operator << (long data)
	{
		return *this << (DWORD)data;
	}
	
	CWbxTraceFormator& operator << (void *ptr)
	{
		return *this << "0x" << (DWORD)ptr;
	}
	
	CWbxTraceFormator& operator << (BYTE data)
	{
		return *this << (DWORD)data;
	}

	CWbxTraceFormator& operator << (char data)
	{
		return *this << (DWORD)data;
	}
	
	CWbxTraceFormator& operator << (const CQtString &str)
	{
		return *this << str.c_str();
	}
	
	CWbxTraceFormator& operator << (LPCSTR str)
	{
		Add(QTECTRC_FORMAT_TYPE_STRING, 2, (LPVOID)str, (str?strlen(str)+1:0));
		return *this;
	}

	CWbxTraceFormator& operator << (char * str)
	{
		return *this << (LPCSTR)str;
	}
			
	CWbxTraceFormator& operator << (LPBYTE lpData)
	{
		m_lpTempBinary = lpData;
		return *this;
	}

	WORD GetSize()
	{
		return m_size;
	}

	WORD GetFormat()
	{
		return m_format;
	}

	void SerializeTo(LPBYTE lpBuf);

private:
	void Add(WORD type, WORD extra_size, LPVOID param1, DWORD size)
	{
		if (m_count >= 16)
			return;

		m_format = m_format | (type << (2 * m_count));
		m_size += size + extra_size;
		m_param1[m_count] = param1;
		m_param2[m_count] = size;
		m_count++;
	}

	LPBYTE	m_lpTempBinary;
	WORD	m_count;
	WORD	m_format;
	WORD	m_size;
	LPVOID	m_param1[16];
	DWORD	m_param2[16];
};

#define	MAX_PRINTABLE_STRING_LENGTH	500

class  QT_OS_EXPORT CWbxPrintableString
{
public :
	CWbxPrintableString()
	{
		m_buf[0] = 0;
		m_len = 0;
	}

	~CWbxPrintableString()
	{
	}

public:
	inline CWbxPrintableString& operator << (unsigned long l);

	inline CWbxPrintableString& operator << (LPCTSTR lpsz);
	
	inline operator LPCSTR()
	{
		return m_buf;
	}
	inline operator LPSTR()
	{
		return m_buf;
	}


private:
	WORD	m_len;
	char	m_buf[MAX_PRINTABLE_STRING_LENGTH + 1];
};

////////////////////////////////////////
// MapFile
//
class QT_OS_EXPORT CWbxMapFile
{
public:
	CWbxMapFile();
	virtual ~CWbxMapFile();
	int Open(char* file_name, DWORD file_size);	// 0: failed, 1: created, 2: opened
	void Close();

	LPVOID Lock(DWORD dwMilliseconds = 5000);
	void Unlock();

protected:
	void Cleanup();

#ifdef WIN32
	HANDLE		m_map_handle;
	LPBYTE		m_map_view_data;
	HANDLE		m_mutex_handle;
#else
	char*	m_file_name;
	DWORD	m_file_size;
	LPBYTE	m_data_buffer;
#endif //UNIX
};

class QT_OS_EXPORT CWbxTraceMgrFile : public CWbxMapFile
{
public:
	CWbxTraceMgrFile();
	virtual ~CWbxTraceMgrFile();

	BOOL Open();
	BOOL IsOpen() { return m_bOpen; }

	BOOL Add(WbxTraceMgrItem* pItem);
	static int GetList(WbxTraceMgrItem* pTable, int size);

protected:
	BOOL m_bOpen;
};

class QT_OS_EXPORT CWbxTraceFile : public CWbxMapFile
{
public:
	CWbxTraceFile(BYTE type, BYTE ver, DWORD static_size, DWORD dynamic_size, DWORD log_size);
	CWbxTraceFile();
	virtual ~CWbxTraceFile() {}

	BOOL Open(DWORD pid);

	void FuncEnter(char* str);
	void FuncLeave();

	LPVOID LockStatic();
	void UnlockStatic();
	BOOL WriteStaticByte(DWORD offset, BYTE b);
	BOOL WriteStaticByteOr(DWORD offset, BYTE b);
	BOOL WriteStaticByteUpd(DWORD offset, char b);
	BOOL WriteStaticWord(DWORD offset, WORD w);
	BOOL WriteStaticWordUpd(DWORD offset, short s);
	BOOL WriteStaticDWord(DWORD offset, DWORD dw);
	BOOL WriteStaticDWordUpd(DWORD offset, int i);
	BOOL WriteStaticString(DWORD offset, char* str);

	int WriteTrace(int area, BYTE level, BYTE event, CWbxTraceFormator& formator);

	DWORD GetFileSize();
	BOOL GetTraceBuffer(LPBYTE lpBuffer, DWORD dwSize);

	void Save2File();

	static void BSWriteUShort(LPVOID pBuf, long off, unsigned short s);
	static void BSWriteULong(LPVOID pBuf, long off, unsigned long l);
	static unsigned short BSReadUShort(LPVOID pBuf, long off);
	static unsigned long BSReadULong(LPVOID pBuf, long off);

protected:
	void UpdateLastModifiedTick(LPVOID lpData);

	CWbxTraceMgrFile m_mgr_map_file;

	BYTE	m_trace_buffer[1024];
	//int	m_idx;
	BOOL	m_bOpen;
	BOOL	m_bWrite;
	DWORD	m_dwStartOffset;
	DWORD	m_dwStaticSize;
	DWORD	m_dwLogSize;
	DWORD	m_dwTraceSize;
};

#if defined QT_WIN32 
#if defined (_LIB) || defined (QT_OS_BUILD_LIB) || !defined (_TP_EXPORTS)
	#pragma message("************QTEC TRACE LIB MODEL *************")
	#define QTECTRC_IMPLEMENT_MODULE(type, ver) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, 0, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_STATIC(type, ver, ss) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC(type, ver, ss, ds) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC_LOG(type, ver, ss, ds, ls) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, ls)
	#define QTECTRC_DECLARE_MODULE(type) extern CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type
#else 
#if defined (_USRDLL) || defined (QT_OS_BUILD_DLL)
	#pragma message("************QTEC TRACE DLL MODEL *************")
	#define QTECTRC_IMPLEMENT_MODULE(type, ver) CWbxTraceFile  __declspec(dllexport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, 0, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_STATIC(type, ver, ss) CWbxTraceFile  __declspec(dllexport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC(type, ver, ss, ds) CWbxTraceFile __declspec(dllexport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC_LOG(type, ver, ss, ds, ls) CWbxTraceFile __declspec(dllexport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, ls)
	#define QTECTRC_DECLARE_MODULE(type) extern CWbxTraceFile __declspec(dllexport) g_wbx_tracefile_##type
#else 
	#pragma message("************QTEC TRACE EXEC MODEL *************")
	#define QTECTRC_IMPLEMENT_MODULE(type, ver) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, 0, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_STATIC(type, ver, ss) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC(type, ver, ss, ds) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC_LOG(type, ver, ss, ds, ls) CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, ls)
	#define QTECTRC_DECLARE_MODULE(type) extern CWbxTraceFile __declspec(dllimport) g_wbx_tracefile_##type
#endif // _USRDLL || QT_OS_BUILD_DLL
#endif // _LIB || QT_OS_BUILD_LIB
#else
	#define QTECTRC_IMPLEMENT_MODULE(type, ver) CWbxTraceFile g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, 0, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_STATIC(type, ver, ss) CWbxTraceFile g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, 0, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC(type, ver, ss, ds) CWbxTraceFile g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, 0)
	#define QTECTRC_IMPLEMENT_MODULE_DYNAMIC_LOG(type, ver, ss, ds, ls) CWbxTraceFile g_wbx_tracefile_##type(QTECTRC_MODULE_TYPE_##type, ver, ss, ds, ls)
	#define QTECTRC_DECLARE_MODULE(type) extern  CWbxTraceFile  g_wbx_tracefile_##type
#endif // !QT_WIN32


#define QTECTRC_FUNC_ENTER(type, str) g_wbx_tracefile_##type.FuncEnter(str)
#define QTECTRC_FUNC_LEAVE(type) g_wbx_tracefile_##type.FuncLeave()

#define QTECTRC_WRITE_BSW	CWbxTraceFile::BSWriteUShort
#define QTECTRC_WRITE_BSD	CWbxTraceFile::BSWriteULong
#define QTECTRC_READ_BSW		CWbxTraceFile::BSReadUShort
#define QTECTRC_READ_BSD		CWbxTraceFile::BSReadULong

#define QTECTRC_LOCK_S(type) g_wbx_tracefile_##type.LockStatic()
#define QTECTRC_UNLOCK_S(type) g_wbx_tracefile_##type.UnlockStatic()
#define QTECTRC_WRITE_SB(type, off, data) g_wbx_tracefile_##type.WriteStaticByte(off, data)
#define QTECTRC_WRITE_SB_OR(type, off, data) g_wbx_tracefile_##type.WriteStaticByteOr(off, data)
#define QTECTRC_WRITE_SB_UPD(type, off, data) g_wbx_tracefile_##type.WriteStaticByteUpd(off, data)
#define QTECTRC_WRITE_SW(type, off, data) g_wbx_tracefile_##type.WriteStaticWord(off, data)
#define QTECTRC_WRITE_SW_UPD(type, off, data) g_wbx_tracefile_##type.WriteStaticWordUpd(off, data)
#define QTECTRC_WRITE_SD(type, off, data) g_wbx_tracefile_##type.WriteStaticDWord(off, data)
#define QTECTRC_WRITE_SD_UPD(type, off, data) g_wbx_tracefile_##type.WriteStaticDWordUpd(off, data)
#define QTECTRC_WRITE_SS(type, off, data) g_wbx_tracefile_##type.WriteStaticString(off, data)
#define QTECTRC_WRITE(type, level, event, data) { \
	CWbxTraceFormator formator; \
	g_wbx_tracefile_##type.WriteTrace(0, level, event, formator << data); \
}
#define QTECTRC_WRITE_LOG(type, level, event, data) { \
	CWbxTraceFormator formator; \
	g_wbx_tracefile_##type.WriteTrace(1, level, event, formator << data); \
	g_wbx_tracefile_##type.WriteTrace(0, level, event, formator); \
}
#define QTECTRC_SAVE2FILE(type) g_wbx_tracefile_##type.Save2File()

class QT_OS_EXPORT CWbxTraceArchive
{
public:
	CWbxTraceArchive(char* file_path);
	virtual ~CWbxTraceArchive();

	int WriteTrace(BYTE type, BYTE level, BYTE event, CWbxTraceFormator& formator);

protected:
	FILE*	m_fp;
	BYTE	m_trace_buffer[1024];
};

#define QTECTRC_DECLARE_ARCHIVE() extern CWbxTraceArchive g_wbx_trace_archive

#define QTECTRC_ARCHIVE(type, level, event, data) { \
	CWbxTraceFormator formator; \
	g_wbx_trace_archive.WriteTrace(type, level, event, formator << data); \
}

#endif //__QTECTRACE_H_
