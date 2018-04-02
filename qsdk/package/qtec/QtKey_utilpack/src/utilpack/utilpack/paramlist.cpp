
#include "QtBase.h"
#include "paramlist.h"

CQtMetaData::CQtMetaData()
{
	m_nType = QT_DATA_UNKNOW;
	m_pszStr = NULL;
	m_pRaw = NULL;
}

// CQtMetaData::CQtMetaData(QT_META_DATA_TYPE nType)
// {
// 	m_nType = nType;
// 	switch(m_nType)
// 	{
// 	case QT_DATA_STRING:
// 		m_pszStr = NULL;
// 		break;
// 	case QT_DATA_RAW:
// 		m_pRaw = NULL;
// 		break;
// 	case QT_DATA_UNKNOW:
// 		m_pszStr = NULL;
// 		m_pRaw = NULL;
// 		break;
// 
// 	default:
// 		break;
// 	}
// }

CQtMetaData::~CQtMetaData()
{
	switch(m_nType)
	{
	case QT_DATA_STRING:
		delete []m_pszStr;
		break;
	case QT_DATA_RAW:
		delete []m_pRaw;
		break;
	default:
		break;
	}
}

QtResult CQtMetaData::Set(char c)
{
	QT_ASSERTE_RETURN(QT_DATA_BYTE == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_BYTE;
	m_cVal = c;
	return QT_OK;
}

QtResult CQtMetaData::Set(short i)
{
	QT_ASSERTE_RETURN(QT_DATA_SHORT == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_SHORT;
	m_sVal = i;
	return QT_OK;
}

QtResult CQtMetaData::Set(int i)
{
	QT_ASSERTE_RETURN(QT_DATA_INT == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_INT;
	m_iVal = i;
	return QT_OK;
}

QtResult CQtMetaData::Set(long l)
{
	QT_ASSERTE_RETURN(QT_DATA_LONG == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_LONG;
	m_lVal = l;
	return QT_OK;
}

QtResult CQtMetaData::Set(WORD w)
{
	QT_ASSERTE_RETURN(QT_DATA_WORD == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_WORD;
	m_wVal = w;
	return QT_OK;
}

QtResult CQtMetaData::Set(DWORD dw)
{
	QT_ASSERTE_RETURN(QT_DATA_DWORD == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_DWORD;
	m_dwVal = dw;
	return QT_OK;
}

QtResult CQtMetaData::Set(float f)
{
	QT_ASSERTE_RETURN(QT_DATA_FLOAT == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_FLOAT;
	m_fVal = f;
	return QT_OK;
}

QtResult CQtMetaData::Set(char* str)
{
	QT_ASSERTE_RETURN(QT_DATA_STRING == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_STRING;
	if(m_pszStr)
		delete []m_pszStr;
	if(str)
	{
		int len = strlen(str);
		m_pszStr = new char[len + 1];
		QT_ASSERTE_RETURN(m_pszStr, QT_ERROR_OUT_OF_MEMORY);
		strcpy(m_pszStr, str);
	}
	else
		m_pszStr = NULL;
	return QT_OK;
}

QtResult CQtMetaData::Set(BYTE* lpData, DWORD dwSize)
{
	QT_ASSERTE_RETURN(QT_DATA_RAW == m_nType || m_nType == QT_DATA_UNKNOW, QT_ERROR_INVALID_ARG);
	if(m_nType == QT_DATA_UNKNOW)
		m_nType = QT_DATA_RAW;
	if(m_pRaw)
		delete []m_pRaw;
	if(lpData && dwSize)
	{
		m_pRaw = new BYTE[dwSize + sizeof(dwSize)];
		QT_ASSERTE_RETURN(m_pRaw, QT_ERROR_OUT_OF_MEMORY);
		memcpy(m_pRaw, &dwSize, sizeof(dwSize));
		memcpy(m_pRaw + sizeof(dwSize), lpData, dwSize);
	}
	else
		m_pRaw = NULL;

	return QT_OK;
}

QT_META_DATA_TYPE CQtMetaData::GetType() const
{
	return m_nType;
}

QtResult CQtMetaData::Get(char& c) const
{
	QT_ASSERTE_RETURN(QT_DATA_BYTE == m_nType, QT_ERROR_INVALID_ARG);
	c = m_cVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(short& i) const
{
	QT_ASSERTE_RETURN(QT_DATA_SHORT == m_nType, QT_ERROR_INVALID_ARG);
	i = m_sVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(int& i) const
{
	QT_ASSERTE_RETURN(QT_DATA_INT == m_nType, QT_ERROR_INVALID_ARG);
	i = m_iVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(long& l) const
{
	QT_ASSERTE_RETURN(QT_DATA_LONG == m_nType, QT_ERROR_INVALID_ARG);
	l = m_lVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(WORD& w) const
{
	QT_ASSERTE_RETURN(QT_DATA_WORD == m_nType, QT_ERROR_INVALID_ARG);
	w = m_wVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(DWORD& dw) const
{
	QT_ASSERTE_RETURN(QT_DATA_DWORD == m_nType, QT_ERROR_INVALID_ARG);
	dw = m_dwVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(float& f) const
{
	QT_ASSERTE_RETURN(QT_DATA_FLOAT == m_nType, QT_ERROR_INVALID_ARG);
	f = m_fVal;
	return QT_OK;
}

QtResult CQtMetaData::Get(const char*& str) const
{
	QT_ASSERTE_RETURN(QT_DATA_STRING == m_nType, QT_ERROR_INVALID_ARG);
	str = m_pszStr;
	return QT_OK;
}

QtResult CQtMetaData::Get(LPBYTE& lpData, DWORD& dwSize)const
{
	QT_ASSERTE_RETURN(QT_DATA_RAW == m_nType, QT_ERROR_INVALID_ARG);
	if(m_pRaw)
	{
		memcpy(&dwSize, m_pRaw, sizeof(dwSize));
		lpData = m_pRaw + sizeof(dwSize);
	}
	else
	{
		lpData = NULL;
		dwSize = 0;
	}
	return QT_OK;
}

QtResult CQtMetaData::GetStreamLength(DWORD& dwLength) const
{
	QT_ASSERTE_RETURN(m_nType != QT_DATA_UNKNOW, QT_ERROR_FAILURE);
	dwLength = sizeof(BYTE);
	switch(m_nType)
	{
	case QT_DATA_BYTE:
		dwLength += sizeof(m_cVal);
		break;
	case QT_DATA_SHORT:
		dwLength += sizeof(m_sVal);
		break;
	case QT_DATA_INT:
		dwLength += sizeof(m_iVal);
		break;
	case QT_DATA_LONG:
		dwLength += sizeof(m_lVal);
		break;
	case QT_DATA_WORD:
		dwLength += sizeof(m_wVal);
		break;
	case QT_DATA_DWORD:
		dwLength += sizeof(m_dwVal);
		break;
	case QT_DATA_FLOAT:
		dwLength += sizeof(m_fVal);
		break;
	case QT_DATA_STRING:
		dwLength += (sizeof(WORD) + (m_pszStr?strlen(m_pszStr):0));
		break;
	case QT_DATA_RAW:
	{
		dwLength += sizeof(DWORD);
		DWORD dwSize = 0;
		if(m_pRaw)
			memcpy(&dwSize, m_pRaw, sizeof(dwSize));
		dwLength += (sizeof(DWORD) + dwSize); 
	}
	break;
	default:
		QT_ASSERTE_RETURN(FALSE, QT_ERROR_FAILURE);
		break;
	}
	return QT_OK;	
}

QtResult CQtMetaData::StreamTo(CQtByteStreamNetwork& os) const
{
	QT_ASSERTE_RETURN(m_nType != QT_DATA_UNKNOW, QT_ERROR_FAILURE);
	
	os<<(BYTE)m_nType;
	switch(m_nType)
	{
	case QT_DATA_BYTE:
		os<<m_cVal;
		break;
	case QT_DATA_SHORT:
		os<<m_sVal;
		break;
	case QT_DATA_INT:
		os<<m_iVal;
		break;
	case QT_DATA_LONG:
		os<<m_lVal;
		break;
	case QT_DATA_WORD:
		os<<m_wVal;
		break;
	case QT_DATA_DWORD:
		os<<m_dwVal;
		break;
	case QT_DATA_FLOAT:
		os<<m_fVal;
		break;
	case QT_DATA_STRING:
		os<<m_pszStr;
		break;
	case QT_DATA_RAW:
	{
		DWORD dwSize = 0;
		if(m_pRaw)
			memcpy(&dwSize, m_pRaw, sizeof(dwSize));
		os<<dwSize;
		if(dwSize > 0)
		os.Write(m_pRaw + sizeof(dwSize), dwSize);
	}
	break;
	default:
		QT_ASSERTE_RETURN(FALSE, QT_ERROR_FAILURE);
		break;
	}
	BOOL bGood = os.IsGood();
	QT_ASSERTE_RETURN(bGood, QT_ERROR_FAILURE);
	return QT_OK;	
}

QtResult CQtMetaData::StreamFrom(CQtByteStreamNetwork& is)
{
	BYTE bType;
	is>>bType;
	m_nType = (QT_META_DATA_TYPE)bType;
	QT_ASSERTE_RETURN(m_nType != QT_DATA_UNKNOW, QT_ERROR_FAILURE);
	switch(m_nType)
	{
	case QT_DATA_BYTE:
		is>>m_cVal;
		break;
	case QT_DATA_SHORT:
		is>>m_sVal;
		break;
	case QT_DATA_INT:
		is>>m_iVal;
		break;
	case QT_DATA_LONG:
		is>>m_lVal;
		break;
	case QT_DATA_WORD:
		is>>m_wVal;
		break;
	case QT_DATA_DWORD:
		is>>m_dwVal;
		break;
	case QT_DATA_FLOAT:
		is>>m_fVal;
		break;
	case QT_DATA_STRING:
		{
			WORD len;
			is>>len;
			if(len)
			{
				m_pszStr = new char[len + 1];
				QT_ASSERTE_RETURN(m_pszStr, QT_ERROR_OUT_OF_MEMORY);
				is.Read(m_pszStr, len);
				m_pszStr[len] = 0;
			}
			else
				m_pszStr = NULL;
		}
		break;
	case QT_DATA_RAW:
	{
		DWORD dwSize = 0;
		is>>dwSize;
		if(dwSize > 0)
		{
			m_pRaw = new BYTE[dwSize + sizeof(dwSize)];
			QT_ASSERTE_RETURN(m_pRaw, QT_ERROR_OUT_OF_MEMORY);
			memcpy(m_pRaw, &dwSize, sizeof(dwSize));
			is.Read(m_pRaw + sizeof(dwSize), dwSize);
		}
		else
			m_pRaw = NULL;
	}
	break;
	default:
		QT_ASSERTE_RETURN(FALSE, QT_ERROR_FAILURE);
		break;
	}
	BOOL bGood = is.IsGood();
	QT_ASSERTE_RETURN(bGood, QT_ERROR_FAILURE);
	return QT_OK;	
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex)
{
	m_dwIndex = dwIndex;
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex,char c)
{
	m_dwIndex = dwIndex;
	m_value.Set(c);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, short s)
{
	m_dwIndex = dwIndex;
	m_value.Set(s);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, int i)
{
	m_dwIndex = dwIndex;
	m_value.Set(i);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, long l)
{
	m_dwIndex = dwIndex;
	m_value.Set(l);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, WORD w)
{
	m_dwIndex = dwIndex;
	m_value.Set(w);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, DWORD dw)
{
	m_dwIndex = dwIndex;
	m_value.Set(dw);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, float f)
{
	m_dwIndex = dwIndex;
	m_value.Set(f);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, char* str)
{
	m_dwIndex = dwIndex;
	m_value.Set(str);
}

CQtParameterByIndex::CQtParameterByIndex(DWORD dwIndex, LPBYTE lpData, DWORD dwSize)
{
	m_dwIndex = dwIndex;
	m_value.Set(lpData, dwSize);
}

CQtParameterByIndex::~CQtParameterByIndex()
{
}

QtResult CQtParameterByIndex::GetStreamLength(DWORD& dwLength) const
{
	QT_ASSERTE_RETURN(m_dwIndex != 0, QT_ERROR_FAILURE);
	QtResult ret = m_value.GetStreamLength(dwLength);
	if(QT_FAILED(ret))
		return ret;
	dwLength += sizeof(m_dwIndex);
	return QT_OK;
}

QtResult CQtParameterByIndex::StreamTo(CQtByteStreamNetwork& os) const
{
	QT_ASSERTE_RETURN(m_dwIndex != 0, QT_ERROR_FAILURE);
	os<<m_dwIndex;
	return m_value.StreamTo(os);
}

QtResult CQtParameterByIndex::StreamFrom(CQtByteStreamNetwork& is)
{
	is >> m_dwIndex;
	return m_value.StreamFrom(is);
}

CQtParameterList::CQtParameterList()
{
}

CQtParameterList::~CQtParameterList()
{
	Clear();
}

void CQtParameterList::Clear()
{
	for(map<DWORD,CQtParameterByIndex*>::iterator it = m_param_list.begin(); it != m_param_list.end(); it++)
	{
		CQtParameterByIndex* pParam = it->second;
		QT_ASSERTE_RETURN_VOID(pParam);
		pParam->ReleaseReference();
	}
	m_param_list.clear();
}

QtResult CQtParameterList::AddParameter(CQtParameterByIndex* pParam)
{
	QT_ASSERTE_RETURN(pParam, QT_ERROR_NULL_POINTER);
	DWORD dwIndex = pParam->GetIndex();
	map<DWORD,CQtParameterByIndex*>::iterator it = m_param_list.find(dwIndex);
	if(it != m_param_list.end())
	{
		CQtParameterByIndex* pOld = it->second;
		pOld->ReleaseReference();
	}
	pParam->AddReference();
	m_param_list[dwIndex] = pParam;
	return QT_OK;
}
QtResult CQtParameterList::GetParameter(DWORD dwIndex, const CQtParameterByIndex*& pParam) const
{
	map<DWORD,CQtParameterByIndex*>::const_iterator it = m_param_list.find(dwIndex);
	if(it != m_param_list.end())
	{
		pParam = it->second;
		return QT_OK;
	}
	return QT_ERROR_NOT_FOUND;
}

QtResult CQtParameterList::RemoveParameter(DWORD dwIndex)
{
	map<DWORD,CQtParameterByIndex*>::iterator it = m_param_list.find(dwIndex);
	if(it != m_param_list.end())
	{
		CQtParameterByIndex* pOld = it->second;
		pOld->ReleaseReference();
		m_param_list.erase(it);
		return QT_OK;
	}
	return QT_ERROR_NOT_FOUND;
}

QtResult CQtParameterList::GetStreamLength(DWORD& dwLength) const
{
	dwLength = sizeof(DWORD);

	DWORD dwTmp;
	QtResult ret;

	for(map<DWORD,CQtParameterByIndex*>::const_iterator it = m_param_list.begin(); it != m_param_list.end(); it++)
	{
		CQtParameterByIndex* pParam = it->second;
		QT_ASSERTE_RETURN(pParam, QT_ERROR_NULL_POINTER);
		ret = pParam->GetStreamLength(dwTmp);
		if(QT_FAILED(ret))
			return ret;
		dwLength += dwTmp;
	}
	return QT_OK;
}

QtResult CQtParameterList::StreamTo(CQtByteStreamNetwork& os) const
{
	DWORD dwCount = m_param_list.size();
	os<<dwCount;

	QtResult ret;
	for(map<DWORD,CQtParameterByIndex*>::const_iterator it = m_param_list.begin(); it != m_param_list.end(); it++)
	{
		CQtParameterByIndex* pParam = it->second;
		QT_ASSERTE_RETURN(pParam, QT_ERROR_NULL_POINTER);
		ret = pParam->StreamTo(os);
		QT_ASSERTE_RETURN(QT_SUCCEEDED(ret), ret); 
		dwCount--;
	}
	QT_ASSERTE_RETURN(0 == dwCount, QT_ERROR_FAILURE);
	return QT_OK;
}

QtResult CQtParameterList::StreamFrom(CQtByteStreamNetwork& is)
{
	DWORD dwCount;
	is>>dwCount;
	if(dwCount == 0)
		return QT_OK;

	QtResult ret;
	for(; dwCount != 0; dwCount--)
	{
		CQtParameterByIndex* pParam = new CQtParameterByIndex();
		QT_ASSERTE_RETURN(pParam, QT_ERROR_OUT_OF_MEMORY);
		ret = pParam->StreamFrom(is);
		if(QT_SUCCEEDED(ret))
			AddParameter(pParam);
		else
		{
			delete pParam;
			return ret;
		}
	}
	return QT_OK;
}
