/*------------------------------------------------------*/
/* Provider a common class for version number                   */
/*                                                      */
/* QtVersion.h                                           */
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

#ifndef QTVERSION_H
#define QTVERSION_H

#include "QtStdCpp.h"
#include "QtMessageBlock.h"
#include "QtByteStream.h"
#ifndef QT_WIN32
	#include <netinet/in.h>
#else
	#include <Winsock2.h>
#endif


using namespace std;


#define GET_BYTE(dwVal, idx)  ((dwVal << (8*idx) >> 24))
#define MAX_SEG_NUM 4

class QT_OS_EXPORT CQtVersion
{
	BYTE m_byteArr[MAX_SEG_NUM];
	DWORD m_dwValue;
	CQtString m_strValue;
	
	void Split(const CQtString &raw, const CQtString &spliter, std::list<CQtString> &result)
	{
		string::size_type idx = 0, idxPre = 0;
		while(idx != string::npos){
			idx = raw.find_first_of(spliter, idxPre);
			if(idx == string::npos){
				CQtString out = raw.substr(idxPre);
				result.push_back(out);
				break;
			}

			CQtString out = raw.substr(idxPre, idx-idxPre);
			result.push_back(out);

			idxPre = idx + spliter.size();
		}
	}

	CQtString IntToStr(DWORD val)
	{
		char buf[12] = {0}; //can hold the bigest 32bit integer 4,294,967,295.
		int i = 11;
		do{
			--i;
			buf[i] = val % 10 + '0';
			val /= 10;
		}while(val);
		
		CQtString s(buf+i);
		return s;
	}

	void IntToByteArr(DWORD val, BYTE *btArr)
	{
		for(int i = 0; i < MAX_SEG_NUM; i++){
			btArr[i] = GET_BYTE(val, i);
		}
	}

	DWORD StrToInt(const CQtString &val)
	{
		DWORD i = 0, num = 0;
		for(; i < val.length(); i++){
			char c = val[i];
			if(isspace(c))
				continue;
			if(!isdigit(c))
				break;
			num *= 10;
			num += c - '0';
		}

		return num;
	}

	CQtString ToStr()
	{
		//if(m_strValue.empty()){
			CQtString s;
			s.reserve(16);
			s.append(IntToStr(m_byteArr[0]));
			s.append(".");
			s.append(IntToStr(m_byteArr[1]));
			s.append(".");
			s.append(IntToStr(m_byteArr[2]));
			s.append(".");
			s.append(IntToStr(m_byteArr[3]));

			m_strValue.assign(s);
		//}
		
		return m_strValue;
	}
public:
	CQtVersion() : m_dwValue(0)
	{
		*(DWORD*)m_byteArr = 0;
		m_strValue = ToStr();
	}

	CQtVersion(const char* strVal)
		: m_dwValue(0)
		, m_strValue(strVal)
	{
		*(DWORD*)m_byteArr = 0;
		if(m_strValue.find(".") == string::npos){
			m_dwValue = StrToInt(m_strValue);
			IntToByteArr(m_dwValue, m_byteArr);
			m_strValue.assign("");
			ToStr();
		}else{
			std::list<CQtString> valList;
			Split(strVal, ".", valList);
			while(valList.size() < MAX_SEG_NUM){
				valList.push_back(".0");
				m_strValue.append(".0");
			}

			int i = 0;
			while(valList.size()){
				CQtString &s = valList.front();
				int tmp = StrToInt(s);
				m_byteArr[i] = tmp;
				valList.pop_front();
				i++;
				if(i >= MAX_SEG_NUM)
					break;
			}

			DWORD dwVal = 0;
			for(i = 0; i < MAX_SEG_NUM; i++){
				dwVal <<= 8;
				dwVal += m_byteArr[i];
			}
			m_dwValue = dwVal;
		}
	}
	
	CQtVersion(DWORD dwVal)
		: m_dwValue(dwVal) 
		, m_strValue("")
	{
		IntToByteArr(dwVal, m_byteArr);
		m_strValue = ToStr();
	}

	CQtVersion(const CQtVersion &rhs)
		: m_dwValue(rhs.m_dwValue)
		, m_strValue(rhs.m_strValue)
	{
		*(DWORD*)m_byteArr = *(DWORD*)rhs.m_byteArr;
	}

	CQtVersion& operator = (const CQtVersion &rhs)
	{
		m_dwValue = rhs.m_dwValue;
		m_strValue = rhs.m_strValue;
		*(DWORD*)m_byteArr = *(DWORD*)rhs.m_byteArr;
		return *this;
	}

	operator CQtString(){
		return m_strValue;
	}

	CQtString GetStringFormat() const { return m_strValue; }
	DWORD GetIntegerFormat() const { return m_dwValue; }

	bool operator < (const CQtVersion &rhs) 
	{
		return m_dwValue < rhs.m_dwValue;
	}

	bool operator == (const CQtVersion &rhs)
	{
		return m_dwValue == rhs.m_dwValue;
	}

	bool operator != (const CQtVersion &rhs)
	{
		return m_dwValue != rhs.m_dwValue;
	}
	
	bool operator <= (const CQtVersion &rhs)
	{
		return (m_dwValue) <= (rhs.m_dwValue);
	}

	BYTE GetByteValue(int index) //index start from 0, from left to right.
	{
		if(index < 0)
			index = 0;
		index %= MAX_SEG_NUM;

		return m_byteArr[index];
	}

	QtResult Encode( CQtMessageBlock& mbBlock ) const
	{
		CQtByteStreamNetwork bsStream( mbBlock );
		bsStream << m_dwValue;
		
		if ( !bsStream.IsGood() )
		{
			return QT_ERROR_FAILURE;
		}
		
		return QT_OK;
	}

	QtResult Decode( CQtMessageBlock& mbBlock )
	{
		CQtByteStreamNetwork bsStream( mbBlock );
		bsStream >> m_dwValue;
		IntToByteArr(m_dwValue, m_byteArr);
		m_strValue = ToStr();
		
		if ( !bsStream.IsGood() )
		{
			return QT_ERROR_FAILURE;
		}
		
		return QT_OK;
	}

};

struct UserJoinInfoExt
{
	CQtVersion m_clientVersion;
	CQtVersion m_requiredVersion;
	DWORD m_profile;
public:
	UserJoinInfoExt() : m_profile(0) {}
};

#endif //QTVERSION_H
