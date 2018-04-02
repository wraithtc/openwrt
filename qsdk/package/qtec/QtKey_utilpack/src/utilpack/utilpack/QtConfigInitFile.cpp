
#include "QtBase.h"
#include "QtConfigInitFile.h"

#ifndef  QT_QTEC_UNIFIED_TRACE
extern unsigned char get_string_param(char* group, char* item_key, char* item_value, unsigned long len);
extern char* get_qtec_home_dir();
extern void set_qtec_home_env(char* home_env);
#endif //!  QT_QTEC_UNIFIED_TRACE

extern void set_qtec_config_file_name(const CQtString &file);
extern void refresh_trace_config();

CQtConfigInitFile::CQtConfigInitFile()
{
}

CQtConfigInitFile::~CQtConfigInitFile()
{
}

QtResult CQtConfigInitFile::InitWithFileName(const CQtString &aFileName)
{
	return QT_OK;
}

int CQtConfigInitFile::
GetIntParam(const CQtString &aGroup,  const CQtString &aKey, int aDefault)
{
	char str_value[256];
	if (get_string_param(
			const_cast<char*>(aGroup.c_str()), 
			const_cast<char*>(aKey.c_str()), 
			str_value, 
			sizeof(str_value)))
	{
		int i;
#if defined QT_WIN32 || defined QT_PORT_CLIENT
		sscanf(str_value, "%d", &i);
#else
		i = strtol( str_value, ( char **)NULL, 10);
		if( i == LONG_MAX || i == LONG_MIN) {
			i = aDefault;
		}
#endif
		return i;
	}
	else
		return aDefault;
}

DWORD CQtConfigInitFile::
GetDwordParam(const CQtString &aGroup, const CQtString &aKey, DWORD aDefault)
{
	char str_value[256];
	if (get_string_param(
			const_cast<char*>(aGroup.c_str()), 
			const_cast<char*>(aKey.c_str()), 
			str_value, 
			sizeof(str_value)))
	{
		DWORD i;
#if defined QT_WIN32 || defined QT_PORT_CLIENT
		sscanf(str_value, "%lu", &i);
#else
		i = strtoul( str_value, ( char **)NULL, 10);
		if( i == ULONG_MAX) {
			i = aDefault;
		}
#endif
		return i;
	}
	else
		return aDefault;
}

WORD CQtConfigInitFile::
GetWordParam(const CQtString &aGroup, const CQtString &aKey, WORD aDefault)
{
	char str_value[256];
	if (get_string_param(
			const_cast<char*>(aGroup.c_str()), 
			const_cast<char*>(aKey.c_str()), 
			str_value, 
			sizeof(str_value)))
	{
		WORD i;
		sscanf(str_value, "%hu", &i);
		return i;
	}
	else
		return aDefault;
}

CQtString CQtConfigInitFile::
GetStringParam(const CQtString &aGroup, const CQtString &aKey, CQtString aDefault)
{
	char str_value[256];
	if (get_string_param(
			const_cast<char*>(aGroup.c_str()), 
			const_cast<char*>(aKey.c_str()), 
			str_value, 
			sizeof(str_value)))
	{
		return CQtString(str_value);
	}
	else
		return aDefault;
}

BOOL CQtConfigInitFile::
GetBoolParam(const CQtString &aGroup, const CQtString &aKey, BOOL aDefault)
{
	char str_value[256];
	if (get_string_param(
			const_cast<char*>(aGroup.c_str()), 
			const_cast<char*>(aKey.c_str()), 
			str_value, 
			sizeof(str_value)))
	{
		return strcasecmp(str_value, "TRUE") == 0;
	}
	else
		return aDefault;
}

CQtString CQtConfigInitFile::GetQtecHomeDir()
{
	char *pszHome = get_qtec_home_dir();
	return CQtString(pszHome);
}

void CQtConfigInitFile::SetQtecHomeEnv(const CQtString &aHome)
{
	set_qtec_home_env(const_cast<char*>(aHome.c_str()));
}

void CQtConfigInitFile::SetQtecConfigFileName(const CQtString &file)
{
	set_qtec_config_file_name(file);
	refresh_trace_config();
	QT_INFO_TRACE("CQtConfigInitFile::SetWebexConfigFileName, file name " << file);
}
