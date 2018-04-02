/*------------------------------------------------------*/
/* Debug utilities(trace, etc.)                         */
/*                                                      */
/* QtDebug.h                                            */
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

#ifndef QTDEBUG_H
#define QTDEBUG_H

#include "QtDefines.h"

#ifdef QT_BIND_WITH_EUREKA
#define QT_DISABLE_TRACE
#endif

#ifndef QT_DISABLE_TRACE
#include "QtTraceFromT120.h"

#if defined MACOS && !defined MachOSupport
/*  #include "attrace.h"
  #define QT_ERROR_TRACE      INFOTRACE
  #define QT_WARNING_TRACE    INFOTRACE
  #define QT_INFO_TRACE       INFOTRACE
  #define QT_STATE_TRACE      INFOTRACE
  #define QT_CLASSFUNC_TRACE */
//Open trace for MAC
#define QT_ERROR_TRACE(str)      QT_T120TRACE(0, str)
#define QT_WARNING_TRACE(str)    QT_T120TRACE(10, str)
#define QT_INFO_TRACE(str)       QT_T120TRACE(20, str)
#define QT_STATE_TRACE(str)      QT_T120TRACE(21, str)
#define QT_CLASSFUNC_TRACE(str)  CQtT120FuncTracer theFUNCTRACE(str);

#else
#if (defined (QT_WIN32) && defined (QT_MMP)&& !defined QT_OUTPUT_TO_FILE && !defined QT_QTEC_UNIFIED_TRACE)
#define QT_ERROR_TRACE(str)      QT_T120TRACE(0, str)
  #define QT_WARNING_TRACE(str)    QT_T120TRACE(1, str)
  #define QT_INFO_TRACE(str)       QT_T120TRACE(2, str)
  #define QT_STATE_TRACE(str)      QT_T120TRACE(3, str)
  #define QT_CLASSFUNC_TRACE(str)  (void *)0//CQtT120FuncTracer theFUNCTRACE(str);
#else
#define QT_ERROR_TRACE(str)      QT_T120TRACE(0, str)
#define QT_WARNING_TRACE(str)    QT_T120TRACE(10, str)
#define QT_INFO_TRACE(str)       QT_T120TRACE(20, str)
#define QT_STATE_TRACE(str)      QT_T120TRACE(21, str)
#define QT_CLASSFUNC_TRACE(str)  CQtT120FuncTracer theFUNCTRACE(str);
#endif //for client trace
#endif //MACOS

#define QT_ERROR_TRACE_THIS(str)    QT_ERROR_TRACE(str << " this=" << this)
#define QT_WARNING_TRACE_THIS(str)  QT_WARNING_TRACE(str << " this=" << this)
#define QT_INFO_TRACE_THIS(str)     QT_INFO_TRACE(str << " this=" << this)
#define QT_STATE_TRACE_THIS(str)    QT_STATE_TRACE(str << " this=" << this)

#define QT_PTT_TRACE(str)			QT_T120PTT_TRACE(str)

#else//!QT_DISABLE_TRACE

#ifdef QT_BIND_WITH_EUREKA

#define LIBUTIL_EXPORT 
#define INFINITE -1
#define THREAD_HANDLE QT_THREAD_HANDLE
#include "platform.h"
#include "t120trace.h"
#include "QtStdCpp.h"

inline T120_Trace::Text_Formator & operator << (T120_Trace::Text_Formator&o,const CQtString &str)
{
        return (o<<str.c_str());
}

#define QT_ERROR_TRACE(str)                     ERRTRACE(str)
#define QT_WARNING_TRACE(str)                   WARNINGTRACE(str) 
#define QT_INFO_TRACE(str)                      INFOTRACE(str)
#define QT_STATE_TRACE(str) 
#define QT_CLASSFUNC_TRACE(str) 

#define QT_ERROR_TRACE_THIS(str)                QT_ERROR_TRACE(str << " this=" << this)
#define QT_WARNING_TRACE_THIS(str)              QT_WARNING_TRACE(str << " this=" << this) 
#define QT_INFO_TRACE_THIS(str)                 QT_INFO_TRACE(str << " this=" << this) 
#define QT_STATE_TRACE_THIS(str) 
#define QT_PTT_TRACE(str)			QT_T120PTTTRACE(str)

#else //QT_BIND_WITH_EUREKA

#define QT_ERROR_TRACE(str) 
#define QT_WARNING_TRACE(str) 
#define QT_INFO_TRACE(str) 
#define QT_STATE_TRACE(str) 
#define QT_CLASSFUNC_TRACE(str) 

#define QT_ERROR_TRACE_THIS(str) 
#define QT_WARNING_TRACE_THIS(str) 
#define QT_INFO_TRACE_THIS(str) 
#define QT_STATE_TRACE_THIS(str) 
#define QT_PTT_TRACE(str)			QT_T120PTTTRACE(str)

#endif //QT_BIND_WITH_EUREKA

#endif // QT_DISABLE_TRACE

/*!
	the definition about the a base object to trace object life and numbers
*/

#ifdef _TRACE_OBJECT

#include "timer.h"
#include <map>
#include <list>
#include "QtStdCpp.h"

#ifdef _OUT_TO_CONSOLE

#include <iostream>
using namespace std;

#endif //_OUT_TO_CONSOLE
#endif

namespace wbx
{

	void QT_OS_EXPORT ObjectTracer();
	class QT_OS_EXPORT IObject
	{
	public:
		virtual DWORD life() = 0;
		virtual ~IObject(){}
#ifdef _TRACE_OBJECT
		struct ObjectGElement{
			CQtString m_GName;		/*! the object group name */
			ObjectGElement(CQtString &name): m_GName(name){}
			bool operator<(const ObjectGElement &right) const{
				if(m_GName.compare(right.m_GName) > 0)
					return true;
				else
					return false;
			}
		};

		struct ObjectElement
		{
			CQtString m_GName;		/*! the object group name */
			ObjectElement(CQtString &name): m_GName(name){}
			typedef std::map<DWORD, IObject *> Object_Map;
			Object_Map m_ObjectList; 
		private:
			ObjectElement();
			ObjectElement(const ObjectElement &);
			ObjectElement & operator=(const ObjectElement &);
		};
		typedef std::map< ObjectGElement, ObjectElement * > Object_G_Map;
		static Object_G_Map *m_pObjectMap;/*! all IObject will be registered here when constructed and unregistered when destroyed */
#endif
		static void DumpObject()
		{

#ifdef _TRACE_OBJECT
			if(!m_pObjectMap)
				return ;
			DWORD i = 0, j = 0;
#ifdef _OUT_TO_CONSOLE
			cout<<"Begin IObject::DumpObject>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[" << m_pObjectMap->size() <<"]" <<endl;
#else
			QT_INFO_TRACE("Begin IObject::DumpObject>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[" << m_pObjectMap->size() <<"]");
#endif//_OUT_TO_CONSOLE
			Object_G_Map::iterator gIt = m_pObjectMap->begin();
			for(; gIt != m_pObjectMap->end(); ++gIt)
			{
				++i;
#ifdef _OUT_TO_CONSOLE
				cout<<"\t["<<i << "] Begin--" << gIt->first.m_GName << " numbers = " << gIt->second->m_ObjectList.size() << endl; 
#else
				QT_INFO_TRACE("\t[" << i << "] Begin--" << gIt->first.m_GName << " numbers = " << gIt->second->m_ObjectList.size()); 
#endif //_OUT_TO_CONSOLE

				ObjectElement *pEle = gIt->second;
				if(!pEle)
					continue;
				ObjectElement::Object_Map::iterator elIt = pEle->m_ObjectList.begin();
				for(; elIt != pEle->m_ObjectList.end(); ++elIt)
				{
					++j;
					if(!elIt->second)
						continue;
#ifdef _OUT_TO_CONSOLE
					cout<<"\t\t["<<i<<"."<<j<<"]-->	Index[" << elIt->first<<"] life = " << elIt->second->life()<< "s" << endl;
#else
					QT_INFO_TRACE("\t\t["<<i<<"."<<j<<"]-->	Index[" << elIt->first<<"] life = " << elIt->second->life()<< "s");
#endif //_OUT_TO_CONSOLE
				}

#ifdef _OUT_TO_CONSOLE
				cout<<"\tEnd--" << gIt->first.m_GName << endl; 
#else
				QT_INFO_TRACE("End--" << gIt->first.m_GName); 
#endif //_OUT_TO_CONSOLE
			}
#ifdef _OUT_TO_CONSOLE
			cout<<"End IObject::DumpObject<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
#else
			QT_INFO_TRACE("End IObject::DumpObject<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
#endif //_OUT_TO_CONSOLE
#endif//_TRACE_OBJECT
		}
	};


	template <typename T, typename MutexType>
		class CObject : public IObject
	{

#ifndef _TRACE_OBJECT
		virtual DWORD life() 
		{
			return 0;
		}

#else 
		virtual DWORD life() 
		{
			return m_Life.elapsed_sec();
		}

		static DWORD m_sCount;		/*! it should can trace the number of the T objects */
		static MutexType *m_sLock;	/*! it should protect the count thread safe if use it in multi-threads system*/
		DWORD m_dwIdx;				/*! the object index */
		static CQtString *m_spGroupName;
		ticker m_Life;
	
		struct auto_lock{
			MutexType &m_mutex;
			auto_lock(MutexType &mutex):m_mutex(mutex)
			{
				m_mutex.Lock();
			}
			~auto_lock()
			{
				m_mutex.UnLock();
			}
		};
	public:
		CObject()
		{
			if(!m_sLock)
				m_sLock = new MutexType;
			QT_ASSERTE_RETURN_VOID(m_sLock);

			auto_lock lock(*m_sLock);
			m_dwIdx = ++m_sCount;
			if(!m_spGroupName)
			{
				char szBuff[2048];
				snprintf(szBuff, sizeof(szBuff), "CObject< %s, %s >", typeid(T).name(), typeid(MutexType).name());
				m_spGroupName = new CQtString(szBuff);
			}
			QT_ASSERTE(m_spGroupName);
			if(m_spGroupName)
				Register(*m_spGroupName);	

#ifdef _OUT_TO_CONSOLE
			cout<<"CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount << ", this = 0x" << hex << this << endl;
#else
			QT_INFO_TRACE_THIS("CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount );
#endif//_OUT_TO_CONSOLE
		}
		
		CObject(const CObject<T, MutexType> &object)
		{
			
			if(!m_sLock)
				m_sLock = new MutexType;
			QT_ASSERTE_RETURN_VOID(m_sLock);
			auto_lock lock(*m_sLock);
			m_dwIdx = ++m_sCount;
			QT_ASSERTE(m_spGroupName);
			if(m_spGroupName)
				Register(*m_spGroupName);	

#ifdef _OUT_TO_CONSOLE
			cout<<"CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount << ", this = 0x" << hex << this << endl;
#else
			QT_INFO_TRACE_THIS("CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount );
#endif//_OUT_TO_CONSOLE

		}

		CObject<T, MutexType> & operator=(const CObject<T, MutexType> &object)
		{
			if(!m_sLock)
				m_sLock = new MutexType;
			QT_ASSERTE_RETURN_VOID(m_sLock);
			auto_lock lock(*m_sLock);
			m_dwIdx = ++m_sCount;
			QT_ASSERTE(m_spGroupName);
			if(m_spGroupName)
				Register(*m_spGroupName);	

#ifdef _OUT_TO_CONSOLE
			cout<<"CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount << ", this = 0x" << hex << this << endl;
#else
			QT_INFO_TRACE_THIS("CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount );
#endif//_OUT_TO_CONSOLE
			return *this;
		}

		virtual ~CObject()
		{
			if(!m_sLock)
				m_sLock = new MutexType;
			QT_ASSERTE_RETURN_VOID(m_sLock);
			auto_lock lock(*m_sLock);
			--m_sCount;
			QT_ASSERTE(m_spGroupName);
			if(m_spGroupName)
				Unregister(*m_spGroupName);
#ifdef _OUT_TO_CONSOLE
			cout<<"CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::~CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount << ", lives "<< (DWORD)m_Life.elapsed_sec() << "s, this = 0x" << hex << this << endl;
#else
			QT_INFO_TRACE_THIS("CObject<" << typeid(T).name() << ", "<<typeid(MutexType).name() << ">::~CObject() index = " << m_dwIdx << ", objects' number = " << m_sCount << ", lives "<< (DWORD)m_Life.elapsed_sec() << "s,");
#endif //_OUT_TO_CONSOLE
		}

	private:
		void Register(CQtString &group_name)
		{
			if(!m_pObjectMap)
				m_pObjectMap = new IObject::Object_G_Map;
			QT_ASSERTE_RETURN_VOID(m_pObjectMap);

			IObject::ObjectGElement el(group_name);
			IObject::Object_G_Map::iterator it = m_pObjectMap->find(el);
			IObject::ObjectElement *pObjEle = NULL;
			if(it == m_pObjectMap->end())
			{
				pObjEle = new ObjectElement(group_name);
				(*m_pObjectMap)[el] = pObjEle;
			}
			else
			{
				pObjEle = it->second;
			}
			QT_ASSERTE_RETURN_VOID(pObjEle);
			QT_ASSERTE_RETURN_VOID(pObjEle->m_ObjectList.find(m_dwIdx) == pObjEle->m_ObjectList.end());
			pObjEle->m_ObjectList[m_dwIdx] = this;
		}
		
		void Unregister(CQtString &group_name)
		{
			QT_ASSERTE_RETURN_VOID(m_pObjectMap);

			IObject::ObjectGElement el(group_name);
			IObject::Object_G_Map::iterator it = m_pObjectMap->find(el);
			QT_ASSERTE_RETURN_VOID(it != m_pObjectMap->end());
			QT_ASSERTE_RETURN_VOID(it->second);

			IObject::ObjectElement *pObjEle = it->second;
			QT_ASSERTE_RETURN_VOID(pObjEle);
			
			IObject::ObjectElement::Object_Map::iterator objIt = pObjEle->m_ObjectList.find(m_dwIdx);
			QT_ASSERTE_RETURN_VOID(objIt != pObjEle->m_ObjectList.end());
			pObjEle->m_ObjectList.erase(objIt);
			if(pObjEle->m_ObjectList.size() == 0)
			{
				delete pObjEle;
				m_pObjectMap->erase(it);
				delete m_spGroupName;
				m_spGroupName = NULL;
			}
			if(m_pObjectMap->size() == 0)
			{
				delete m_pObjectMap;
				m_pObjectMap = NULL;
			}
		}
#endif//_TRACE_OBJECT
	};

#ifdef _TRACE_OBJECT

	template <typename T, typename MutexType>
		DWORD CObject<T, MutexType>::m_sCount = 0;
	template <typename T, typename MutexType>
		MutexType *CObject<T, MutexType>::m_sLock;
	template <typename T, typename MutexType>
		CQtString *CObject<T, MutexType>::m_spGroupName = NULL;

	
#define OBJECT_TRACER		IObject::DumpObject()
#else
#define OBJECT_TRACER ObjectTracer()
#endif//_TRACE_OBJECT

}
#endif // QTDEBUG_H
