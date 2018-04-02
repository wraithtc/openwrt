/*------------------------------------------------------*/
/* a twin wrapper that can supply a mechanism           */
/*                                                      */
/* twin_wrap.h									        */
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

#ifndef _TWIN_WRAP_INC_ 
#define _TWIN_WRAP_INC_

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "QtBase.h"

//!supply a twin call mechanism£¬not supply parameters now!
/**
 * example:
 * CThreadMutex myMutex;
 * CTwinWrap<CThreadMutex, void (CThreadMutex::*)(), void (CThreadMutex::*)()> 
 * locks_(myMutex, &CThreadMutex::Lock, &CThreadMutex::Unlock);
 */
/**
 * @param T object instance
 * @param PointerAcquire  
 * @param PointerRelease  
 */
template<
			typename T, 
			typename PointerAcquire = void (T::*)(), 
			typename PointerRelease = void (T::*)()
			>
struct CTwinWrap
{
    //!Constructor£¬supply a first function call
    /**
    *@param impl   executor object
    *@param acquire call when constructor
    *@param release call when destructor
    *@return void
    */
    CTwinWrap(
             T &_impl ,              
             PointerAcquire acquire,         
             PointerRelease release        
             ):m_impl(&_impl), m_acquire(acquire), m_release(release)
    {
        (m_impl->*m_acquire)();
    }

    //!Destructor
    ~CTwinWrap()
    {
        (m_impl->*m_release)();
    }


private:
    T* m_impl;     /*!< object instance */                 
    PointerAcquire m_acquire;       /*!< the function's instance that call when constuctor */
    PointerRelease m_release;       /*!< the function's instance that call when destructor */
};

//!supply a twin call mechanism, for two 'C' function or functional object£¬not supply parameters now!
template< typename PointerAcquire, typename PointerRelease >
struct CTwinFunctorWrap
{
	CTwinFunctorWrap(PointerAcquire *lpAcquire, PointerRelease *lpRelease)
		:m_lpRelease(lpRelease)
	{
		if(lpAcquire)
			(*lpAcquire)();
	}
	~CTwinFunctorWrap()
	{
		if(m_lpRelease)
			(*m_lpRelease)();
	}
private:
	PointerRelease * m_lpRelease;       /*!< release function or object */
};

/* 
void *lpf = malloc(10);
void *lpf1 = malloc(10);
Res_DestroyFunctor<void *, void (*)(void *), NotNull<void *> > mydes(free, lpf, NotNull<void *>());
Res_DestroyFunctor_ref<void *, void (*)(void *)> mydes_ref(&lpf1, free);
*/

template<typename T>
struct NotNull{
	bool operator()(T val)
	{
		return val != NULL;
	}
};

template<typename _PARAM, typename _FUNC = int (*)(_PARAM),  typename _WorkCondition = NotNull<_PARAM> >
struct Res_DestroyFunctor{
	Res_DestroyFunctor(_PARAM param, _FUNC func, _WorkCondition cond)
		:m_func(func), m_param(param), m_Cond(cond)
	{}

	Res_DestroyFunctor(_PARAM param, _FUNC func)
		:m_func(func), m_param(param)
	{
		m_Cond = NotNull<_PARAM>();
	}

	~Res_DestroyFunctor()
	{
		this->Execute();
	}
	operator _PARAM()
	{
		return m_param;
	}

private:
	void Execute()
	{
		if(m_Cond(m_param))
			m_func((_PARAM)m_param);
	}
	Res_DestroyFunctor();
	Res_DestroyFunctor& operator=(const Res_DestroyFunctor&);
	_FUNC   m_func;
	_PARAM  m_param;
	_WorkCondition m_Cond;
};

template<typename _PARAM, typename _FUNC = int (*)(_PARAM), typename _WorkCondition = NotNull<_PARAM> >
struct Res_DestroyFunctor_ref{
	
	Res_DestroyFunctor_ref(_PARAM *param, _FUNC func, _WorkCondition cond)
		:m_func(func), m_lpParam(param), m_Cond(cond)
	{	}

	Res_DestroyFunctor_ref(_PARAM *param, _FUNC func)
	:m_func(func), m_lpParam(param)
	{
		m_Cond = NotNull<_PARAM>();
	}

	operator _PARAM()
	{
		return *m_lpParam;
	}

	~Res_DestroyFunctor_ref()
	{
		this->Execute();
	}
private:
	void Execute()
	{
		if(m_Cond(*m_lpParam)){
			m_func((_PARAM)*m_lpParam);
			*m_lpParam = 0;
		}
	}
	Res_DestroyFunctor_ref();
	Res_DestroyFunctor_ref& operator=(const Res_DestroyFunctor_ref&);
	_FUNC   m_func;
	_PARAM*  m_lpParam;
	_WorkCondition m_Cond;
};

#endif


