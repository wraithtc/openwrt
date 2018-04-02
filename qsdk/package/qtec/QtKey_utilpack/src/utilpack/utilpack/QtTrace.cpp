
#include "QtTrace.h"
#include <time.h>
#ifdef UNIX
	#include <sys/time.h>
#endif

typedef void (*MyBSSwapProc)(void* pData, int size);
MyBSSwapProc g_mybs_pfnSwap = NULL;

#include <assert.h>
template<typename LockOperator>
class AutoLockWrapper{
	LockOperator *m_pLock;
	void * m_pAppData;
	public:
		AutoLockWrapper(LockOperator *pLock):m_pLock(pLock){
			assert(pLock);
			m_pAppData = m_pLock->Lock();
		}
		void *GetData() const{
			return m_pAppData;
		}

		~AutoLockWrapper()
		{
			m_pLock->Unlock();
		}
};
BOOL MyBSIsLittleEndian()
{
    WORD wTest = 0x55aa;

    return (*(LPBYTE)&wTest == 0xaa);
}

void MyBSLittelEndianSwap(void* pData, int size)
{
	unsigned char *pch_head = (unsigned char*)pData, 
		*pch_tail = (unsigned char*)pData + size - 1;

	for (; pch_head < pch_tail; pch_head++, pch_tail--)
	{
		unsigned char chTemp;
		chTemp = *pch_head;
		*pch_head = *pch_tail;
		*pch_tail = chTemp;
	}
}

void MyBSBigEndianSwap(void* pData, int size)
{
}

void MyBSSwap(void* pData, int size)
{
    if(!g_mybs_pfnSwap)
    {
        if (MyBSIsLittleEndian())
            g_mybs_pfnSwap = MyBSLittelEndianSwap;
        else
            g_mybs_pfnSwap = MyBSBigEndianSwap;
    }

    g_mybs_pfnSwap(pData, size);
}

void CWbxTraceFile::BSWriteUShort(LPVOID pBuf, long off, unsigned short s)
{
	memcpy((unsigned char*)pBuf + off, &s, sizeof(unsigned short));
	MyBSSwap((unsigned char*)pBuf + off, sizeof(unsigned short));
}

void CWbxTraceFile::BSWriteULong(LPVOID pBuf, long off, unsigned long l)
{
	memcpy((unsigned char*)pBuf + off, &l, sizeof(unsigned long));
	MyBSSwap((unsigned char*)pBuf + off, sizeof(unsigned long));
}

unsigned short CWbxTraceFile::BSReadUShort(LPVOID pBuf, long off)
{
	unsigned short s;
	memcpy(&s, (unsigned char*)pBuf + off, sizeof(unsigned short));
	MyBSSwap(&s, sizeof(unsigned short));

	return s;
}

unsigned long CWbxTraceFile::BSReadULong(LPVOID pBuf, long off)
{
	unsigned long l;
	memcpy(&l, (unsigned char*)pBuf + off, sizeof(unsigned long));
	MyBSSwap(&l, sizeof(unsigned long));

	return l;
}

void CWbxTraceFormator::SerializeTo(LPBYTE lpBuf)
{
	int offset = 0;

	WORD f2 = m_format;
	for (WORD i = 0; i < m_count; i++)
	{
		switch (f2 & 3)
		{
		case QTECTRC_FORMAT_TYPE_NUM:
			CWbxTraceFile::BSWriteULong(lpBuf, offset, (DWORD)m_param1[i]);
			offset += 4;
			break;

		case QTECTRC_FORMAT_TYPE_STRING:
		case QTECTRC_FORMAT_TYPE_BINARY:
			CWbxTraceFile::BSWriteUShort(lpBuf, offset, (DWORD)m_param2[i]);
			offset += 2;
			memcpy(lpBuf + offset, m_param1[i], (DWORD)m_param2[i]);
			offset += m_param2[i];
			break;
		}

		f2 = f2 >> 2;
	}
}

////////////////////////////////////////
// MapFile
//
#ifdef WIN32
CWbxMapFile::CWbxMapFile()
{
	m_map_handle = NULL;
	m_map_view_data = NULL;
	m_mutex_handle = NULL;
}

CWbxMapFile::~CWbxMapFile()
{
	Cleanup();
}

int CWbxMapFile::Open(char* file_name, DWORD file_size)
{
	if (m_map_view_data != NULL)
		//TRC_INFO("CATMapFile::Open, already open");
		return 2;

	if (file_name == NULL)
		//TRC_INFO("CATMapFile::Open, invalid parameter");
		return 0;

	BOOL bSecure = TRUE;

	SECURITY_DESCRIPTOR sd;
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{
		//TRC_INFO("CATMapFile::Open, create security descriptor failed, error=" << GetLastError());
		bSecure = FALSE;
	}
	else if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE))
	{
		//TRC_INFO("CATMapFile::Open, set security descriptor dacl failed, error=" << GetLastError());
		bSecure = FALSE;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = TRUE;
	char*   mutexName = new char [lstrlen(file_name) + 10];
	if (mutexName == NULL)
	{
		//TRC_INFO("CATMapFile::Open, memory allocate failed");
		Cleanup();
		return 0;
	}
	wsprintf(mutexName, "%sMutex", file_name);
	m_mutex_handle = CreateMutex((bSecure ? &sa : NULL), FALSE, mutexName);
	delete[] mutexName;

	if (m_mutex_handle == NULL)
	{
		//TRC_INFO("CATMapFile::Open, create mutex failed, error=" << GetLastError());
		Cleanup();
		return 0;
	}

	AutoLockWrapper<CWbxMapFile> myLock(this);
	BOOL bCreate = FALSE;
	m_map_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, file_name);

	if (m_map_handle == NULL && file_size != 0)
	{
		//TRC_INFO("CATMapFile::Open, open failed, now create it");
		m_map_handle = CreateFileMapping((HANDLE)0xFFFFFFFF, (bSecure ? &sa : NULL), 
			PAGE_READWRITE, 0, file_size, file_name);
		bCreate = TRUE;
	}

	if (m_map_handle == NULL)
	{
		//TRC_INFO("CATMapFile::Open, create file mapping failed, error=" << GetLastError());
		int err = GetLastError();
		Cleanup();
		return 0;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		bCreate = FALSE;

	m_map_view_data = (LPBYTE)MapViewOfFile(m_map_handle, 
		FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);

	if (m_map_view_data == NULL)
	{
		//TRC_INFO("CATMapFile::CATMapFile, MapViewOfFile failed");
		Cleanup();
		return 0;
	}

	if (bCreate)
	{
		//TRC_INFO("CATMapFile::Open, map file is created");
	}
	else
	{
		//TRC_INFO("CATMapFile::Open, map file is opened");
	}

	return (bCreate ? 1 : 2);
}

void CWbxMapFile::Close()
{
	//TRC_FUNC("CATMapFile::Close");

	Cleanup();
}

void CWbxMapFile::Cleanup()
{
	if (m_map_view_data)
		UnmapViewOfFile(m_map_view_data);
	m_map_view_data = NULL;

	if (m_map_handle)
		CloseHandle(m_map_handle);
	m_map_handle = NULL;

	if (m_mutex_handle)
		CloseHandle(m_mutex_handle);
	m_mutex_handle = NULL;
}

LPVOID CWbxMapFile::Lock(DWORD dwMilliseconds)
{
	if (m_mutex_handle == NULL)
	{
		//TRC_INFO("CATMapFile::Lock, m_mutex_handle == NULL");
		return NULL;
	}

	if (WaitForSingleObject(m_mutex_handle, dwMilliseconds) == WAIT_TIMEOUT)
	{
		//TRC_INFO("CATMapFile::Lock, wait timeout");
		return NULL;
	}

	return m_map_view_data;
}

void CWbxMapFile::Unlock()
{
	BOOL bRet = ReleaseMutex(m_mutex_handle);
}

#else // WIN32

CWbxMapFile::CWbxMapFile()
{
	m_file_name = NULL;
	m_file_size = 0;
	m_data_buffer = NULL;
}

CWbxMapFile::~CWbxMapFile()
{
	Cleanup();
}

int CWbxMapFile::Open(char* file_name, DWORD file_size)
{
	if (!file_name || !file_size)
		return 0;

	m_data_buffer = new BYTE [file_size];
	if (!m_data_buffer)
		return 0;

	m_file_name = new char [strlen(file_name) + 1];
	if (!m_file_name)
	{
		delete []m_data_buffer;
		return 0;
	}

	strcpy(m_file_name, file_name);
	m_file_size = file_size;

	return 1;
}

void CWbxMapFile::Close()
{
	if (!m_file_name || !m_data_buffer)
		return;

	FILE* fp = fopen(m_file_name, "wb");
	if (fp)
	{
		fwrite(m_data_buffer, m_file_size, 1, fp);
		fclose(fp);
	}
}

LPVOID CWbxMapFile::Lock(DWORD dwMilliseconds)
{
	return m_data_buffer;
}

void CWbxMapFile::Unlock()
{
}

void CWbxMapFile::Cleanup()
{
	if (m_data_buffer)
	{
		delete []m_data_buffer;
		m_data_buffer = NULL;
	}

	if (m_file_name)
	{
		delete []m_file_name;
		m_file_name = NULL;
	}
}

#endif

CWbxTraceMgrFile::CWbxTraceMgrFile()
{
	m_bOpen = FALSE;
}

CWbxTraceMgrFile::~CWbxTraceMgrFile()
{
}

BOOL CWbxTraceMgrFile::Open()
{
	if (m_bOpen)
		return TRUE;

	LPVOID lpData;
	int ret = CWbxMapFile::Open("WbxTraceManager", QTECTRC_MGR_LIST_SIZE * sizeof(WbxTraceMgrItem));
	if (ret == 0)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	lpData = myLock.GetData();
	if (!lpData)
	{
		Close();
		return FALSE;
	}

	if (ret == 1)
		memset(lpData, 0, QTECTRC_MGR_LIST_SIZE * sizeof(WbxTraceMgrItem));

	m_bOpen = TRUE;
	return TRUE;
}

BOOL IsProcessAlive(DWORD pid)
{
#ifdef WIN32
	HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (handle)
	{
		CloseHandle(handle);
		return TRUE;
	}
#endif
	return FALSE;
}

BOOL CWbxTraceMgrFile::Add(WbxTraceMgrItem* pItem)
{
	if (!IsOpen())
	{
		if (!Open())
			return FALSE;
	}

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	WbxTraceMgrItem* pItem2 = (WbxTraceMgrItem*)lpData;
	int idx = -1;
//	char file_name[100];
	for (int i = 0; i < QTECTRC_MGR_LIST_SIZE; i++, pItem2++)
	{
		DWORD pid = CWbxTraceFile::BSReadULong((LPBYTE)&pItem2->pid, 0);
		if (pid)
		{
			if (!IsProcessAlive(pid))
				pid = pItem2->pid = 0;
		}
		if (idx == -1 && pid == 0)
			idx = i;
	}
	if (idx < 0)
	{
		return FALSE;
	}
	pItem2 = (WbxTraceMgrItem*)lpData + idx;
	CWbxTraceFile::BSWriteULong((LPBYTE)&pItem2->pid, 0, pItem->pid);

	return TRUE;
}

int CWbxTraceMgrFile::GetList(WbxTraceMgrItem* pTable, int size)
{
	CWbxMapFile mgr_map_file;

	int ret = mgr_map_file.Open("WbxTraceManager", 0);
	if (ret == 0)
		return 0;
	AutoLockWrapper<CWbxMapFile> myLock(&mgr_map_file);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return 0;

	WbxTraceMgrItem* pItem = (WbxTraceMgrItem*)lpData;
	int i, n;
	//CWbxMapFile tmp_mapfile;
//	char file_name[100];
	for (i = 0, n = 0; i < QTECTRC_MGR_LIST_SIZE && n < size; i++, pItem++)
	{
		if (pItem->pid == 0)
			continue;
		DWORD pid = CWbxTraceFile::BSReadULong((LPBYTE)&pItem->pid, 0);
		if (!IsProcessAlive(pid))
		{
			pItem->pid = 0;
			continue;
		}
		//sprintf(file_name, "WbxTraceFile_%u", pid);
		//if (tmp_mapfile.Open(file_name, 0) == 0)
		//	continue;

		//tmp_mapfile.Close();
		pTable->pid = pid;
		pTable++;
		n++;
	}
	return n;
}

CWbxTraceFile::CWbxTraceFile(BYTE type, BYTE version, DWORD static_size, DWORD dynamic_size, DWORD log_size)
{
	if (type >= QTECTRC_MODULE_MAX_NUM)
		return;

	m_bOpen = FALSE;
	int ret;

	if (!m_mgr_map_file.Open())
		return;

	if (!dynamic_size)
		dynamic_size = 32 * 1024;

	if (!log_size)
		log_size = 1024;

	m_dwStaticSize = static_size;
	m_dwTraceSize = dynamic_size;
	m_dwLogSize = log_size;

	DWORD pid;
#ifdef WIN32
	pid = GetCurrentProcessId();
#endif
#ifdef UNIX
	pid = getpid();
#endif

	char file_name[100];
	sprintf(file_name, "WbxTraceFile_%u", pid);
	ret = CWbxMapFile::Open(file_name, QTECTRC_MAX_ALLOC_FILE_SIZE);
	if (ret == 0)
	{
		Close();
		return;
	}

	LPVOID lpData;
	AutoLockWrapper<CWbxMapFile> myLock(this);
	WbxTraceFileHeader* pHeader;
	if (ret == 1)
	{
		WbxTraceMgrItem item;
		item.pid = pid;
		if (!m_mgr_map_file.Add(&item))
		{
			Close();
			return;
		}

		lpData = myLock.GetData();
		if (!lpData)
		{
			Close();
			return;
		}

		memset(lpData, 0, QTECTRC_MAX_ALLOC_FILE_SIZE);

		pHeader = (WbxTraceFileHeader*)lpData;

		pHeader->header_size = sizeof(WbxTraceFileHeader);
#ifdef WIN32
		pHeader->os_type = QTECTRC_OS_TYPE_WIN;
#elif WINCE
		pHeader->os_type = QTECTRC_OS_TYPE_WINCE;
#elif SOLARIS
		pHeader->os_type = QTECTRC_OS_TYPE_SOLARIS;
#elif LINUX
		pHeader->os_type = QTECTRC_OS_TYPE_LINUX;
#elif HPOX
		pHeader->os_type = QTECTRC_OS_TYPE_HPOX;
#elif MACOS
		pHeader->os_type = QTECTRC_OS_TYPE_MAC;
#endif
		pHeader->module_num = QTECTRC_MODULE_MAX_NUM;
		pHeader->thread_num = QTECTRC_STACK_THREAD_MAX_COUNT;

		BSWriteULong((LPBYTE)&pHeader->pid, 0, pid);
		BSWriteULong((LPBYTE)&pHeader->alloc_size, 0, QTECTRC_MAX_ALLOC_FILE_SIZE);
		BSWriteULong((LPBYTE)&pHeader->used_size, 0, 
			sizeof(WbxTraceFileHeader) + sizeof(WbxTraceFileThread) * QTECTRC_STACK_THREAD_MAX_COUNT);

		BSWriteULong((LPBYTE)&pHeader->start_sec, 0, time(NULL));
		DWORD start_msec;
#ifdef WIN32
		SYSTEMTIME systime; GetLocalTime(&systime); start_msec = systime.wMilliseconds;
#elif defined(MACOS)
		struct timeval tp; GetTickCount() / 1000;
#else
		struct timeval tp; gettimeofday(&tp, NULL); start_msec = tp.tv_usec / 1000;
#endif
		BSWriteULong((LPBYTE)&pHeader->start_msec, 0, start_msec);
		BSWriteULong((LPBYTE)&pHeader->start_tick, 0, GetTickCount());
	}
	else
	{
		lpData = myLock.GetData();
		if (!lpData)
		{
			Close();
			return;
		}
		pHeader = (WbxTraceFileHeader*)lpData;
	}

	WbxTraceFileModule* pModule = pHeader->module_table + type;
	if (pModule->status == 0)
	{
		m_dwStartOffset = BSReadULong((LPBYTE)&pHeader->used_size, 0);
		DWORD alloc_size = BSReadULong((LPBYTE)&pHeader->alloc_size, 0);
		DWORD dwRemain = alloc_size - m_dwStartOffset;
		if (dwRemain < m_dwStaticSize + m_dwLogSize + m_dwTraceSize)
		{
			if (dwRemain >= m_dwStaticSize + m_dwLogSize + 1024)
				m_dwTraceSize = dwRemain - (m_dwStaticSize + m_dwLogSize);
			else
			{
				m_dwTraceSize = 0;
				if (dwRemain < m_dwStaticSize + m_dwLogSize)
				{
					if (dwRemain >= m_dwStaticSize + 1024)
						m_dwLogSize = dwRemain - m_dwStaticSize;
					else
					{
						m_dwLogSize = 0;
						if (dwRemain < m_dwStaticSize)
							m_dwStaticSize = 0;
					}
				}
			}
		}

		if (!m_dwStaticSize && !m_dwLogSize && m_dwTraceSize)
		{
			Close();
			return;
		}

		pModule->status = 1;
		pModule->version = version;
		BSWriteULong((LPBYTE)&pModule->start_offset, 0, m_dwStartOffset);
		BSWriteULong((LPBYTE)&pModule->static_size, 0, m_dwStaticSize);
		BSWriteULong((LPBYTE)&pModule->log_size, 0, m_dwLogSize);
		BSWriteULong((LPBYTE)&pModule->trace_size, 0, m_dwTraceSize);

		DWORD dwOffset = m_dwStartOffset + m_dwStaticSize;
		if (m_dwLogSize)
		{
			BSWriteULong((LPBYTE)lpData + dwOffset, 0, m_dwLogSize - 12);
			BSWriteULong((LPBYTE)lpData + dwOffset, 4, 0);
			BSWriteULong((LPBYTE)lpData + dwOffset, 8, 0);
		}
		dwOffset += m_dwLogSize;
		if (m_dwTraceSize)
		{
			BSWriteULong((LPBYTE)lpData + dwOffset, 0, m_dwTraceSize - 12);
			BSWriteULong((LPBYTE)lpData + dwOffset, 4, 0);
			BSWriteULong((LPBYTE)lpData + dwOffset, 8, 0);
		}

		BSWriteULong((LPBYTE)&pHeader->used_size, 0, 
			m_dwStartOffset + m_dwStaticSize + m_dwLogSize + m_dwTraceSize);
		UpdateLastModifiedTick(lpData);
	}
	else
	{
		m_dwStartOffset = BSReadULong((LPBYTE)&pModule->start_offset, 0);
		m_dwStaticSize = BSReadULong((LPBYTE)&pModule->static_size, 0);
		m_dwLogSize = BSReadULong((LPBYTE)&pModule->log_size, 0);
		m_dwTraceSize = BSReadULong((LPBYTE)&pModule->trace_size, 0);
	}

	m_bOpen = TRUE;
	m_bWrite = TRUE;
}

CWbxTraceFile::CWbxTraceFile()
{
	m_mgr_map_file.Open();
	m_bOpen = FALSE;
	m_dwStaticSize = 0;
	m_dwTraceSize = 0;
	m_dwLogSize = 0;
}

void CWbxTraceFile::UpdateLastModifiedTick(LPVOID lpData)
{
	BSWriteULong((LPBYTE)lpData, offsetof(WbxTraceFileHeader, last_modified_tick), GetTickCount());
}

BOOL CWbxTraceFile::Open(DWORD pid)
{
	char file_name[100];
	sprintf(file_name, "WbxTraceFile_%u", pid);
	int ret = CWbxMapFile::Open(file_name, 0);
	if (ret == 0)
		return FALSE;

	m_bOpen = TRUE;
	m_bWrite = FALSE;
	return TRUE;
}

void CWbxTraceFile::FuncEnter(char* str)
{
	if (!m_bOpen || !m_bWrite)
		return;

	DWORD thread_id;
	thread_id = GetCurrentThreadId();

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return;

	WbxTraceFileHeader* pHeader = (WbxTraceFileHeader*)lpData;
	WbxTraceFileThread* pThreadRoot = (WbxTraceFileThread*)((LPBYTE)lpData + pHeader->header_size);
	WbxTraceFileThread* pThread = pThreadRoot;
	int n = -1;
	for (int i = 0; i < pHeader->thread_num; i++, pThread++)
	{
		if (pThread->thread_id == 0)
		{
			if (n == -1)
				n = i;
		}
		else if (BSReadULong((LPBYTE)&pThread->thread_id, 0) == thread_id)
		{
			n = i;
			break;
		}
	}

	if (n >= 0)
	{
		pThread = pThreadRoot + n;
		if (pThread->thread_id == 0)
			BSWriteULong((LPBYTE)&pThread->thread_id, 0, thread_id);

		if (pThread->stack_depth >= QTECTRC_STACK_ITEM_MAX_COUNT)
			BSWriteUShort((LPBYTE)&pThread->stack_additional, 0, 
				BSReadUShort((LPBYTE)&pThread->stack_additional, 0) + 1);
		else
		{
			strncpy(pThread->stack[pThread->stack_depth], str, QTECTRC_STACK_ITEM_MAX_LENGTH - 1);
			pThread->stack_depth++;
		}
	}

}

void CWbxTraceFile::FuncLeave()
{
	if (!m_bOpen || !m_bWrite)
		return;

	DWORD thread_id;
	thread_id = GetCurrentThreadId();

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return;

	WbxTraceFileHeader* pHeader = (WbxTraceFileHeader*)lpData;
	WbxTraceFileThread* pThread = (WbxTraceFileThread*)((LPBYTE)lpData + pHeader->header_size);
	for (int i = 0; i < pHeader->thread_num; i++, pThread++)
	{
		if (BSReadULong((LPBYTE)&pThread->thread_id, 0) == thread_id)
		{
			WORD stack_additional = BSReadUShort((LPBYTE)&pThread->stack_additional, 0);
			if (stack_additional > 0)
				BSWriteUShort((LPBYTE)&pThread->stack_additional, 0, stack_additional - 1);
			else
			{
				if (pThread->stack_depth > 0)
					pThread->stack_depth -= 1;
				if (pThread->stack_depth == 0)
					pThread->thread_id = 0;
			}
			break;
		}
	}

}

LPVOID CWbxTraceFile::LockStatic()
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return NULL;

	LPVOID lpData = Lock();
	if (!lpData)
		return NULL;

	return (LPBYTE)lpData + m_dwStartOffset;
}

void CWbxTraceFile::UnlockStatic()
{
	if (m_bOpen && m_bWrite && m_dwStaticSize)
		Unlock();
}

BOOL CWbxTraceFile::WriteStaticByte(DWORD offset, BYTE b)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 1 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	*((LPBYTE)lpData + m_dwStartOffset + offset) = b;
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticByteOr(DWORD offset, BYTE b)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 1 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	*((LPBYTE)lpData + m_dwStartOffset + offset) |= b;
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticByteUpd(DWORD offset, char c)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 1 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	*((LPBYTE)lpData + m_dwStartOffset + offset) += c;
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticWord(DWORD offset, WORD w)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 2 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	BSWriteUShort((LPBYTE)lpData + m_dwStartOffset, offset, w);
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticWordUpd(DWORD offset, short s)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 2 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	BSWriteUShort((LPBYTE)lpData + m_dwStartOffset, offset, 
		BSReadUShort((LPBYTE)lpData + m_dwStartOffset, offset) + s);
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticDWord(DWORD offset, DWORD dw)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 4 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	BSWriteULong((LPBYTE)lpData + m_dwStartOffset, offset, dw);
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticDWordUpd(DWORD offset, int i)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (offset + 4 > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	BSWriteULong((LPBYTE)lpData + m_dwStartOffset, offset, 
		BSReadULong((LPBYTE)lpData + m_dwStartOffset, offset) + i);
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteStaticString(DWORD offset, char* str)
{
	if (!m_bOpen || !m_bWrite || !m_dwStaticSize)
		return FALSE;

	if (!str)
		str = "";

	int len = strlen(str) + 1;

	if (offset + len > m_dwStaticSize)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	strcpy((char*)lpData + m_dwStartOffset + offset, str);
	UpdateLastModifiedTick(lpData);
	return TRUE;
}

BOOL CWbxTraceFile::WriteTrace(int area, BYTE level, BYTE event, CWbxTraceFormator& formator)
{
	if (!m_bOpen || !m_bWrite)
		return FALSE;

	DWORD dwOffset = m_dwStartOffset + m_dwStaticSize;
	if (area == 0)
	{
		if (m_dwTraceSize == 0)
			return FALSE;
		dwOffset += m_dwLogSize;
	}
	else if (area == 1)
	{
		if (m_dwLogSize == 0)
			return FALSE;
	}
	else
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	WORD wSize = sizeof(WbxTraceBlock);
	LPBYTE lpBuffer = m_trace_buffer + wSize;
	wSize += formator.GetSize();
	if (wSize > sizeof(m_trace_buffer))
	{
		return FALSE;
	}
	formator.SerializeTo(lpBuffer);

	WbxTraceBlock* pBlock = (WbxTraceBlock*)m_trace_buffer;
	BSWriteUShort((LPBYTE)&pBlock->size, 0, wSize);
	pBlock->level = level;
	pBlock->event = event;
	BSWriteULong((LPBYTE)&pBlock->tick, 0, GetTickCount());
#ifdef MACOS	
	BSWriteUShort((LPBYTE)&pBlock->thread_id, 0, (WORD)0);
#else
	BSWriteUShort((LPBYTE)&pBlock->thread_id, 0, (WORD)GetCurrentThreadId());
#endif
	BSWriteUShort((LPBYTE)&pBlock->format, 0, formator.GetFormat());

	LPBYTE lpHeader = (LPBYTE)lpData + dwOffset;

	DWORD dwAvailable = 0;
	DWORD dynamic_size = BSReadULong(lpHeader, 0);
	DWORD dynamic_head = BSReadULong(lpHeader, 4);
	DWORD dynamic_tail = BSReadULong(lpHeader, 8);
//	if (dynamic_size <= dynamic_head || dynamic_size <= dynamic_tail || dynamic_size > 1000000 || dynamic_head > 1000000 || dynamic_tail > 1000000)
//			dwAvailable = dwAvailable / (dwAvailable - dwAvailable);

	if (dynamic_head >= dynamic_tail)
		dwAvailable = dynamic_size - (dynamic_head - dynamic_tail) - 1;
	else
		dwAvailable = dynamic_tail - dynamic_head - 1;

	WORD wTemp;
	while (dwAvailable < wSize)
	{
		if (dynamic_tail < dynamic_size - 1)
			wTemp = BSReadUShort(lpHeader + 12 + dynamic_tail, 0);
		else
		{
			BYTE bTemp[2];
			bTemp[0] = lpHeader[12 + dynamic_tail];
			bTemp[1] = lpHeader[12];
			wTemp = BSReadUShort(bTemp, 0);
		}

		if (wTemp == 0 || wTemp >= 1024)
		{
			Save2File();
			wTemp = 1;
//			wTemp = wTemp / (wTemp - 1);
		}

		dynamic_tail += wTemp;
		if (dynamic_tail >= dynamic_size)
			dynamic_tail -= dynamic_size;
		dwAvailable += wTemp;
	}

	DWORD dwTemp = dynamic_size - dynamic_head;
	if (dwTemp >= wSize)
	{
		memcpy(lpHeader + 12 + dynamic_head, m_trace_buffer, wSize);
		dynamic_head += wSize;
		if (dynamic_head >= dynamic_size)
			dynamic_head -= dynamic_size;
	}
	else
	{
		memcpy(lpHeader + 12 + dynamic_head, m_trace_buffer, dwTemp);
		if(dwTemp < sizeof(m_trace_buffer))
		memcpy(lpHeader + 12, m_trace_buffer + dwTemp, wSize - dwTemp);
		else
			return FALSE;

		dynamic_head = wSize - dwTemp;
	}

	BSWriteULong(lpHeader, 4, dynamic_head);
	BSWriteULong(lpHeader, 8, dynamic_tail);

	UpdateLastModifiedTick(lpData);
	return TRUE;
}

DWORD CWbxTraceFile::GetFileSize()
{
	if (!m_bOpen)
		return 0;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return 0;

	WbxTraceFileHeader* pHeader = (WbxTraceFileHeader*)lpData;
	DWORD dwSize = BSReadULong((LPBYTE)&pHeader->used_size, 0);
	return dwSize;
}

BOOL CWbxTraceFile::GetTraceBuffer(LPBYTE lpBuffer, DWORD dwSize)
{
	if (!m_bOpen)
		return FALSE;

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
		return FALSE;

	WbxTraceFileHeader* pHeader = (WbxTraceFileHeader*)lpData;
	DWORD dwTemp = BSReadULong((LPBYTE)&pHeader->used_size, 0);
	if (dwTemp <= dwSize)
		memcpy(lpBuffer, lpData, dwTemp);
	else
		dwTemp = 0;
	return dwTemp > 0;
}

void FindFile(char* achFile)
{
#ifdef WIN32
	char achPath[MAX_PATH];
	GetTempPath(MAX_PATH, achPath);
	//INFOTRACE("FindFile, temp path=" << achPath);

	FILETIME oldWriteTime;
	int idx = -1;
	for (int i = 1; i <= 5; i++)
	{
		sprintf(achFile, "%swbxtrc%d.dat", achPath, i);
		FILETIME localFTime;
		WIN32_FIND_DATA findbuf;
		HANDLE findhandle = FindFirstFile(achFile, &findbuf);
		if (findhandle == INVALID_HANDLE_VALUE || 
			!FileTimeToLocalFileTime(&findbuf.ftLastWriteTime, &localFTime))
		{
			//INFOTRACE("FindFile, GetFileAttributesEx failed, achFile=" << achFile);
			return;
		}

		if (idx == -1)
		{
			idx = i;
			oldWriteTime = localFTime;
		}
		else if (oldWriteTime.dwHighDateTime > localFTime.dwHighDateTime || 
			oldWriteTime.dwHighDateTime == localFTime.dwHighDateTime && 
			oldWriteTime.dwLowDateTime > localFTime.dwLowDateTime)
		{
			idx = i;
			oldWriteTime = localFTime;
		}
	}
	sprintf(achFile, "%s\\wbxtrc%d.dat", achPath, idx);
#else
	strcpy(achFile, "/tmp/wbxtrc.dat");
#endif
	//INFOTRACE("FindFile return, achFile=" << achFile);
}

void CWbxTraceFile::Save2File()
{
	char achFile[MAX_PATH];
	FindFile(achFile);

	FILE* fp = fopen(achFile, "wb");
	if (fp == NULL)
	{
		//INFOTRACE("Save2File open file failed, achFile=" << achFile);
		return;
	}

	AutoLockWrapper<CWbxMapFile> myLock(this);
	LPVOID lpData = myLock.GetData();
	if (!lpData)
	{
		//INFOTRACE("Save2File, lock failed");
		fclose(fp);
		return;
	}

	WbxTraceFileHeader* pHeader = (WbxTraceFileHeader*)lpData;
	DWORD dwTemp = BSReadULong((LPBYTE)&pHeader->used_size, 0);
	fwrite(lpData, 1, dwTemp, fp);
	fclose(fp);
	//INFOTRACE("Save2File, succeed");
}

CWbxTraceArchive::CWbxTraceArchive(char* file_path)
{
	m_fp = fopen(file_path, "wb");
	if (!m_fp)
		return;

	BYTE buffer[100];
	memcpy(buffer, "QTECARC", 6);

	DWORD pid;
#ifdef WIN32
	pid = GetCurrentProcessId();
#endif
#ifdef UNIX
	pid = getpid();
#endif
	CWbxTraceFile::BSWriteULong(buffer, 6, pid);

	CWbxTraceFile::BSWriteULong(buffer, 10, time(NULL));
	DWORD start_msec;
#ifdef WIN32
	SYSTEMTIME systime; GetLocalTime(&systime); start_msec = systime.wMilliseconds;
#endif
#ifdef UNIX
	struct timeval tp; gettimeofday(&tp, NULL); start_msec = tp.tv_usec / 1000;
#endif
	CWbxTraceFile::BSWriteULong(buffer, 14, start_msec);
	CWbxTraceFile::BSWriteULong(buffer, 18, GetTickCount());

	fwrite(buffer, 22, 1, m_fp);
}

CWbxTraceArchive::~CWbxTraceArchive()
{
	fclose(m_fp);
}

int CWbxTraceArchive::WriteTrace(BYTE type, BYTE level, BYTE event, CWbxTraceFormator& formator)
{
	if (!m_fp)
		return 1;

	WORD wSize = sizeof(WbxTraceBlock);
	LPBYTE lpBuffer = m_trace_buffer + wSize + 1;
	wSize += formator.GetSize();
	if (wSize > sizeof(m_trace_buffer))
		return 1;
	formator.SerializeTo(lpBuffer);

	m_trace_buffer[0] = type;
	WbxTraceBlock* pBlock = (WbxTraceBlock*)m_trace_buffer;
	CWbxTraceFile::BSWriteUShort((LPBYTE)&pBlock->size, 0, wSize);
	pBlock->level = level;
	pBlock->event = event;
	CWbxTraceFile::BSWriteULong((LPBYTE)&pBlock->tick, 0, GetTickCount());
	CWbxTraceFile::BSWriteUShort((LPBYTE)&pBlock->thread_id, 0, (WORD)GetCurrentThreadId());
	CWbxTraceFile::BSWriteUShort((LPBYTE)&pBlock->format, 0, formator.GetFormat());

	fwrite(m_trace_buffer, wSize, 1, m_fp);
	return 0;
}


CWbxPrintableString& CWbxPrintableString::operator << (unsigned long l)
{
	if (m_len + 10 < MAX_PRINTABLE_STRING_LENGTH)
	{
		if (l < 0x10000 || l > 0xffff0000)
			m_len += sprintf((char *)(m_buf + m_len), "%d", l);
		else
			m_len += sprintf((char *)(m_buf + m_len), "0x%lx", l);
	}
	
	return *this;
}

CWbxPrintableString& CWbxPrintableString::operator << (LPCTSTR lpsz)
{
	if (lpsz && *lpsz)
	{
		int len = lstrlen(lpsz);
		
		if (m_len + len > MAX_PRINTABLE_STRING_LENGTH)
			len = MAX_PRINTABLE_STRING_LENGTH - m_len;
		
		memcpy(m_buf + m_len, lpsz, len);
		m_len += len;
		m_buf[m_len] = 0;
	}
	
	return *this;
}
