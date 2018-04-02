/*------------------------------------------------------*/
/* Implement hash map due to the lack of STL            */
/*                                                      */
/* QtHashMapT.h                                         */
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

#ifndef QTHASHMAPT_H
#define QTHASHMAPT_H

#include "QtStdCpp.h"
#include <vector>
#include <functional>

#define _STLP_BEGIN_NAMESPACE namespace cm_std {
#define _STLP_END_NAMESPACE }

_STLP_BEGIN_NAMESPACE

/* 
// define our own allocate, not use std::allocator
template<class _Ty> inline
	_Ty  *QT_Allocate(ptrdiff_t _N, _Ty  *)
	{if (_N < 0)
		_N = 0;
	return ((_Ty  *)operator new(
		(size_t)_N * sizeof (_Ty))); }
		// TEMPLATE FUNCTION QT_Construct
template<class _T1, class _T2> inline
	void QT_Construct(_T1  *_P, const _T2& _V)
	{new ((void  *)_P) _T1(_V); }
		// TEMPLATE FUNCTION QT_Destroy
template<class _Ty> inline
	void QT_Destroy(_Ty  *_P)
	{(_P)->~_Ty();}
inline void QT_Destroy(char  *_P)
	{}
inline void QT_Destroy(wchar_t  *_P)
	{}

template<class _Ty>
	class CQtAllocator {
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef _Ty  *pointer;
	typedef const _Ty  *const_pointer;
	typedef _Ty & reference;
	typedef const _Ty & const_reference;
	typedef _Ty value_type;
	pointer address(reference _X) const
		{return (&_X); }
	const_pointer address(const_reference _X) const
		{return (&_X); }
	pointer allocate(size_type _N, const void * = NULL)
		{return (QT_Allocate((difference_type)_N, (pointer)0)); }
	char  *_Charalloc(size_type _N)
		{return (QT_Allocate((difference_type)_N,
			(char  *)0)); }
	void deallocate(void  *_P, size_type)
		{operator delete(_P); }
	void construct(pointer _P, const _Ty& _V)
		{QT_Construct(_P, _V); }
	void destroy(pointer _P)
		{QT_Destroy(_P); }
	size_type max_size() const
		{size_type _N = (size_type)(-1) / sizeof (_Ty);
		return (0 < _N ? _N : 1); }
	template <class _Tp1> struct rebind {
		typedef CQtAllocator<_Tp1> other;
	  };
	};
template<class _Ty, class _U> inline
	bool operator==(const CQtAllocator<_Ty>&, const CQtAllocator<_U>&)
	{return (true); }
template<class _Ty, class _U> inline
	bool operator!=(const CQtAllocator<_Ty>&, const CQtAllocator<_U>&)
	{return (false); }
		// CLASS allocator<void>
template<> class CQtAllocator<void> {
public:
	typedef void _Ty;
	typedef _Ty  *pointer;
	typedef const _Ty  *const_pointer;
	typedef _Ty value_type;
	};

#define allocator CQtAllocator
*/

#define allocator std::allocator


#define forward_iterator_tag std::forward_iterator_tag
#define pair std::pair
#define equal_to std::equal_to
#define __vector__ std::vector

#define _STLP_CALL
#define _STLP_DEFINE_ARROW_OPERATOR  pointer operator->() const { return &(operator*()); }
//#ifdef QT_MACOS
//	#define _STLP_STD cm_std
//#else
	#define _STLP_STD std
//#endif
#define _STLP_NESTED_TYPE_PARAM_BUG

#ifndef QT_WIN32
  #define _STLP_USE_EXCEPTIONS
#endif // QT_WIN32

# ifdef _STLP_USE_EXCEPTIONS
#   define _STLP_TRY try
#   define _STLP_CATCH_ALL catch(...)
#   define _STLP_THROW(x) throw x
#   define _STLP_RETHROW throw
#   define _STLP_UNWIND(action) catch(...) { action; throw; }
# else
#   define _STLP_TRY 
#   define _STLP_CATCH_ALL if (false)
#    define _STLP_THROW(x)
#   define _STLP_RETHROW {}
#   define _STLP_UNWIND(action) 
# endif

#define __DFL_TMPL_PARAM( classname, defval ) class classname = defval
#define __DFL_TMPL_ARG(classname)  
#define _STLP_DEFAULT_PAIR_ALLOCATOR_SELECT(_Key, _Tp ) \
             class _Alloc = allocator< pair < _Key, _Tp > >

#define  _STLP_SELECT1ST(__x, __y) _Select1st< __x >

template <class _Pair>
struct _Select1st : public std::unary_function<_Pair, typename _Pair::first_type> {
  const typename _Pair::first_type& operator()(const _Pair& __x) const {
    return __x.first;
  }
};

// traits typedefs.
// fbp: those are being used for iterator/const_iterator definitions everywhere
template <class _Tp>
struct _Nonconst_traits;

template <class _Tp>
struct _Const_traits {
  typedef _Tp value_type;
  typedef const _Tp&  reference;
  typedef const _Tp*  pointer;
  typedef _Nonconst_traits<_Tp> _Non_const_traits;
};

template <class _Tp>
struct _Nonconst_traits {
  typedef _Tp value_type;
  typedef _Tp& reference;
  typedef _Tp* pointer;
  typedef _Nonconst_traits<_Tp> _Non_const_traits;
};


// allocator typedefs.
// The fully general version.
#define _STLP_FORCE_ALLOCATORS(a,y) 

template <class _Tp, class _Allocator>
struct _Alloc_traits
{
	typedef _Allocator _Orig;
	// I am sorry for such ugly due to lock of Rebind(). budingc
	typedef allocator<_Tp> allocator_type;
};

#define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __stl_alloc_create(__a,(_Tp*)0)

template <class _Tp1, class _Tp2>
inline allocator<_Tp2>& _STLP_CALL
__stl_alloc_rebind(allocator<_Tp1>& __a, const _Tp2*) {  return (allocator<_Tp2>&)(__a); }
template <class _Tp1, class _Tp2>
inline allocator<_Tp2> _STLP_CALL
__stl_alloc_create(const allocator<_Tp1>&, const _Tp2*) { return allocator<_Tp2>(); }

template <class _Value, class _Tp, class _MaybeReboundAlloc>
class _STLP_alloc_proxy : public _MaybeReboundAlloc {
private:
  typedef _MaybeReboundAlloc _Base;
  typedef _STLP_alloc_proxy<_Value, _Tp, _MaybeReboundAlloc> _Self;
public:
  _Value _M_data;
  inline _STLP_alloc_proxy(const _MaybeReboundAlloc& __a, _Value __p) : _MaybeReboundAlloc(__a), _M_data(__p) {}

  // Unified interface to perform allocate()/deallocate() with limited
  // language support
  // else it is rebound already, and allocate() member is accessible
  inline _Tp* allocate(size_t __n) { 
    return __stl_alloc_rebind(static_cast<_Base&>(*this),(_Tp*)0).allocate(__n,0); 
  }
  inline void deallocate(_Tp* __p, size_t __n) { 
    __stl_alloc_rebind(static_cast<_Base&>(*this),(_Tp*)0).deallocate(__p, __n); 
  }
};

_STLP_END_NAMESPACE

#define QT_ENABLE_HASH_MAP_REPORT

#include "_hashtable.h"
#include "_hashmap.h"

#define CQtHashMapT cm_std::hash_map

#endif // QTHASHMAPT_H
