#ifndef __ParamList__
#define __ParamList__

#include "QtBase.h"
#include "QtByteStream.h"
#include <map>

using namespace std;

enum QT_META_DATA_TYPE
{
	QT_DATA_BYTE,
	QT_DATA_SHORT,
	QT_DATA_INT,
	QT_DATA_LONG,
	QT_DATA_WORD,
	QT_DATA_DWORD,
	QT_DATA_FLOAT,
	QT_DATA_STRING,
	QT_DATA_RAW,
	QT_DATA_UNKNOW
};

class QT_OS_EXPORT CQtMetaData
{
public:
	CQtMetaData();
// 	CQtMetaData(QT_META_DATA_TYPE nType);
	virtual ~CQtMetaData();

public:
	QtResult Set(char c);
	QtResult Set(short i);
	QtResult Set(int i);
	QtResult Set(long l);
	QtResult Set(WORD w);
	QtResult Set(DWORD dw);
	QtResult Set(float f);
	QtResult Set(char* str);
	QtResult Set(BYTE* lpData, DWORD dwSize);

	QtResult Get(char& c) const;
	QtResult Get(short& i) const;
	QtResult Get(int& i) const;
	QtResult Get(long& l) const;
	QtResult Get(WORD& w) const;
	QtResult Get(DWORD& dw) const;
	QtResult Get(float& f) const;
	QtResult Get(const char*& str) const;//get the pointer, so caller should not delete it.
	QtResult Get(LPBYTE& lpData, DWORD& dwSize)const;//get the pointer, so caller should not delete it.

	QT_META_DATA_TYPE GetType() const;

public:
	QtResult GetStreamLength(DWORD& dwLength) const;
	QtResult StreamTo(CQtByteStreamNetwork& os) const;
	QtResult StreamFrom(CQtByteStreamNetwork& is);

/*protected:
	QtResult Set(QT_META_DATA_TYPE nType, LPVOID lpValue, DWORD dwSize) = 0;
	QtResult Get(QT_META_DATA_TYPE& nType, LPVOID& lpValue, DWORD& dwSize) = 0;
*/
	
protected:
	QT_META_DATA_TYPE	m_nType;
	union
	{
		char		m_cVal;
		short       m_sVal;
		int         m_iVal;
		long		m_lVal;
		WORD		m_wVal;
		DWORD		m_dwVal;
		float		m_fVal;
		char*       m_pszStr;
		BYTE*		m_pRaw;
	};	
};

class QT_OS_EXPORT CQtParameterByIndex: public CQtReferenceControlMutilThread// user should use reference number control the lifetime
{
public:
	CQtParameterByIndex(DWORD dwIndex = 0);
	CQtParameterByIndex(DWORD dwIndex,char c);
	CQtParameterByIndex(DWORD dwIndex, short s);
	CQtParameterByIndex(DWORD dwIndex, int i);
	CQtParameterByIndex(DWORD dwIndex, long l);
	CQtParameterByIndex(DWORD dwIndex, WORD w);
	CQtParameterByIndex(DWORD dwIndex, DWORD dw);
	CQtParameterByIndex(DWORD dwIndex, float f);
	CQtParameterByIndex(DWORD dwIndex, char* str);
	CQtParameterByIndex(DWORD dwIndex, LPBYTE lpData, DWORD dwSize);
	virtual ~CQtParameterByIndex();
	DWORD GetIndex() const
	{
		return m_dwIndex;
	}
	CQtMetaData& GetMetaData()
	{
		return m_value;
	}
	const CQtMetaData& GetMetaData() const
	{
		return m_value;
	}

public:
	QtResult GetStreamLength(DWORD& dwLength) const;
	QtResult StreamTo(CQtByteStreamNetwork& os) const;
	QtResult StreamFrom(CQtByteStreamNetwork& is);
	
protected:
	DWORD			m_dwIndex;
	CQtMetaData		m_value;
};

class QT_OS_EXPORT CQtParameterList
{
public:
	CQtParameterList();
	virtual ~CQtParameterList();
	
public:
	QtResult AddParameter(CQtParameterByIndex* pParam);
	//get the pointer, so caller should not delete it.
	QtResult GetParameter(DWORD dwIndex, const CQtParameterByIndex*& pParam) const;
	QtResult RemoveParameter(DWORD dwIndex);

public:
	QtResult GetStreamLength(DWORD& dwLength) const;
	virtual QtResult StreamTo(CQtByteStreamNetwork& os) const;
	virtual QtResult StreamFrom(CQtByteStreamNetwork& is);

protected:
	void Clear();
	
protected:
	map<DWORD,CQtParameterByIndex*>	m_param_list;
};

#endif
