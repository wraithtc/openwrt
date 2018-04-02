/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

/// This file is copied from stlport/stl/_hashtable.c

#ifndef QT_STLP_INTERNAL_HASHTABLE_H
#define QT_STLP_INTERNAL_HASHTABLE_H

// Hashtable class, used to implement the hashed associative containers
// hash_set, hash_map, hash_multiset, and hash_multimap.

_STLP_BEGIN_NAMESPACE
using std::ptrdiff_t;

template <class _ForwardIter, class _Tp, class _Compare, class _Distance>
_ForwardIter __lower_bound(_ForwardIter __first, _ForwardIter __last,
			   const _Tp& __val, _Compare __comp, _Distance*)
{
	_Distance __len = std::distance(__first, __last);
  _Distance __half;
  _ForwardIter __middle;

  while (__len > 0) {
    __half = __len >> 1;
    __middle = __first;
    std::advance(__middle, __half);
    if (__comp(*__middle, __val)) {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
    else
      __len = __half;
  }
  return __first;
}

template <class _Tp>
std::less<_Tp> __less(_Tp* ) { return std::less<_Tp>(); }

template <class _Val>
struct _Hashtable_node
{
  typedef _Hashtable_node<_Val> _Self;
  _Self* _M_next;
  _Val _M_val;
};  

// some compilers require the names of template parameters to be the same
template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
class hashtable;

template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
struct _Hashtable_iterator
{
  typedef hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
          _Hashtable;
  typedef _Hashtable_node<_Val> _Node;

  _Node* _M_cur;
  _Hashtable* _M_ht;

  _Hashtable_iterator(_Node* __n, _Hashtable* __tab) 
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_iterator() {}

  _Node* _M_skip_to_next();
};


template <class _Val, class _Traits, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
struct _Ht_iterator : public _Hashtable_iterator< _Val, _Key,_HF, _ExK,_EqK,_All>
{
  
  typedef _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All> _Base;

  //  typedef _Ht_iterator<_Val, _Nonconst_traits<_Val>,_Key,_HF,_ExK,_EqK,_All> iterator;
  //  typedef _Ht_iterator<_Val, _Const_traits<_Val>,_Key,_HF,_ExK,_EqK,_All> const_iterator;
  typedef _Ht_iterator<_Val, _Traits,_Key,_HF,_ExK,_EqK,_All> _Self;

  typedef hashtable<_Val,_Key,_HF,_ExK,_EqK,_All> _Hashtable;
  typedef _Hashtable_node<_Val> _Node;

  typedef _Val value_type;
  typedef forward_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef typename _Traits::reference reference;
  typedef typename _Traits::pointer   pointer;

  _Ht_iterator(const _Node* __n, const _Hashtable* __tab) :
    _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>((_Node*)__n, (_Hashtable*)__tab) {}
  _Ht_iterator() {}
  _Ht_iterator(const _Ht_iterator<_Val, _Nonconst_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>& __it) : 
    _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>(__it) {}

  reference operator*() const { 
      return this->_M_cur->_M_val; 
  }
  _STLP_DEFINE_ARROW_OPERATOR

  _Self& operator++() {
    _Node* __n = this->_M_cur->_M_next;
    this->_M_cur =  (__n !=0 ? __n : this->_M_skip_to_next());
    return *this;
  }
  inline  _Self operator++(int) {
     _Self __tmp = *this;
    ++*this;
    return __tmp;
  }
};

template <class _Val, class _Traits, class _Traits1, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline bool 
operator==(const _Ht_iterator<_Val, _Traits,_Key,_HF,_ExK,_EqK,_All>& __x, 
	   const _Ht_iterator<_Val, _Traits1,_Key,_HF,_ExK,_EqK,_All>& __y) { 
  return __x._M_cur == __y._M_cur; 
}

//#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE
#if 1
template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline bool 
operator!=(const _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>& __x, 
	   const _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>& __y) { 
  return __x._M_cur != __y._M_cur; 
}
#else

# if (defined (__GNUC__) && (__GNUC_MINOR__ < 8))
template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline bool
operator!=(const _Ht_iterator<_Val, _Const_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>& __x,
           const _Ht_iterator<_Val, _Nonconst_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>& __y) {
  return __x._M_cur != __y._M_cur;
}
# endif

template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline bool 
operator!=(const _Ht_iterator<_Val, _Nonconst_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>& __x, 
	   const _Ht_iterator<_Val, _Const_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>& __y) { 
  return __x._M_cur != __y._M_cur; 
}
#endif

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
template <class _Val, class _Traits, class _Key, class _HF, class _ExK, class _EqK, class _All>
inline _Val* value_type(const _Ht_iterator<_Val, _Traits,_Key,_HF,_ExK,_EqK,_All>&) { return (_Val*) 0; }
template <class _Val, class _Traits, class _Key, class _HF, class _ExK, class _EqK, class _All>
inline forward_iterator_tag iterator_category(const _Ht_iterator<_Val, _Traits,_Key,_HF,_ExK,_EqK,_All>&) { return forward_iterator_tag(); }
template <class _Val, class _Traits, class _Key, class _HF, class _ExK, class _EqK, class _All>
inline ptrdiff_t* distance_type(const _Ht_iterator<_Val,_Traits,_Key,_HF,_ExK,_EqK,_All>&) { return (ptrdiff_t*) 0; }
#endif

#define __stl_num_primes  28

#if 0 // budingc
template <class _Tp>
class _Stl_prime {
public:
  static const size_t _M_list[__stl_num_primes];
};

# if defined (_STLP_USE_TEMPLATE_EXPORT) 
_STLP_EXPORT_TEMPLATE_CLASS _Stl_prime<bool>;
# endif

typedef _Stl_prime<bool> _Stl_prime_type;

#else // !0
class QT_OS_EXPORT _Stl_prime_type {
public:
  static const size_t _M_list[__stl_num_primes];
};

#endif // 0

#ifdef MACOS
/*
template<class _T1, class _T2> inline
void _Construct(_T1 *_P, const _T2& _V)
{
	new ((void  *)_P) _T1(_V);
}
// TEMPLATE FUNCTION _Destroy
template<class _Ty> inline
void _Destroy(_Ty *_P)
{
	_P->~_Ty();//DESTRUCTOR(_Ty, _P);
}
*/
#endif

// Hashtables handle allocators a bit differently than other containers
//  do.  If we're using standard-conforming allocators, then a hashtable
//  unconditionally has a member variable to hold its allocator, even if
//  it so happens that all instances of the allocator type are identical.
// This is because, for hashtables, this extra storage is negligible.  
//  Additionally, a base class wouldn't serve any other purposes; it 
//  wouldn't, for example, simplify the exception-handling code.
template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
class hashtable {
  typedef hashtable<_Val, _Key, _HF, _ExK, _EqK, _All> _Self;
public:
  typedef _Key key_type;
  typedef _Val value_type;
  typedef _HF hasher;
  typedef _EqK key_equal;

  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef forward_iterator_tag _Iterator_category;

  hasher hash_funct() const { return _M_hash; }
  key_equal key_eq() const { return _M_equals; }

private:
  typedef _Hashtable_node<_Val> _Node;

private:
  _STLP_FORCE_ALLOCATORS(_Val, _All)
  typedef typename _Alloc_traits<_Node, _All>::allocator_type _M_node_allocator_type;
  typedef typename _Alloc_traits<void*, _All>::allocator_type _M_node_ptr_allocator_type;
  typedef __vector__<void*, _M_node_ptr_allocator_type> _BucketVector;
public:
  typedef typename _Alloc_traits<_Val,_All>::allocator_type allocator_type;
  allocator_type get_allocator() const { 
    return _STLP_CONVERT_ALLOCATOR((const _M_node_allocator_type&)_M_num_elements, _Val); 
  }
private:
  hasher                _M_hash;
  key_equal             _M_equals;
  _ExK                  _M_get_key;
  _BucketVector         _M_buckets;
  _STLP_alloc_proxy<size_type, _Node, _M_node_allocator_type>  _M_num_elements;
  const _Node* _M_get_bucket(size_t __n) const { return (_Node*)_M_buckets[__n]; }

public:
  typedef _Const_traits<_Val> __const_val_traits;
  typedef _Nonconst_traits<_Val> __nonconst_val_traits;
  typedef _Ht_iterator<_Val, __const_val_traits,_Key,_HF,_ExK,_EqK, _All> const_iterator;
  typedef _Ht_iterator<_Val, __nonconst_val_traits,_Key,_HF,_ExK,_EqK,_All> iterator;
  friend struct _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>;
  friend struct _Ht_iterator<_Val, _Nonconst_traits<_Val>,_Key,_HF,_ExK,_EqK,_All>;
  friend struct _Ht_iterator<_Val, _Const_traits<_Val>,_Key,_HF,_ExK,_EqK, _All>;

public:
  hashtable(size_type __n,
            const _HF&  __hf,
            const _EqK& __eql,
            const _ExK& __ext,
            const allocator_type& __a = allocator_type())
    :
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(__ext),
      _M_buckets(_STLP_CONVERT_ALLOCATOR(__a,void*)),
      _M_num_elements(_STLP_CONVERT_ALLOCATOR(__a,_Node), (size_type)0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(size_type __n,
            const _HF&    __hf,
            const _EqK&   __eql,
            const allocator_type& __a = allocator_type())
    :
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(_ExK()),
      _M_buckets(_STLP_CONVERT_ALLOCATOR(__a,void*)),
      _M_num_elements(_STLP_CONVERT_ALLOCATOR(__a,_Node), (size_type)0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(const _Self& __ht)
    :
      _M_hash(__ht._M_hash),
      _M_equals(__ht._M_equals),
      _M_get_key(__ht._M_get_key),
      _M_buckets(_STLP_CONVERT_ALLOCATOR(__ht.get_allocator(),void*)),
      _M_num_elements((const _M_node_allocator_type&)__ht._M_num_elements, (size_type)0)
  {
    _M_copy_from(__ht);
  }

  _Self& operator= (const _Self& __ht)
  {
    if (&__ht != this) {
      clear();
      _M_hash = __ht._M_hash;
      _M_equals = __ht._M_equals;
      _M_get_key = __ht._M_get_key;
      _M_copy_from(__ht);
    }
    return *this;
  }

  ~hashtable() { clear(); }

  size_type size() const { return _M_num_elements._M_data; }
  size_type max_size() const { return size_type(-1); }
  bool empty() const { return size() == 0; }

  void swap(_Self& __ht)
  {
    _STLP_STD::swap(_M_hash, __ht._M_hash);
    _STLP_STD::swap(_M_equals, __ht._M_equals);
    _STLP_STD::swap(_M_get_key, __ht._M_get_key);
    _M_buckets.swap(__ht._M_buckets);
    _STLP_STD::swap(_M_num_elements, __ht._M_num_elements);
  }

  iterator begin()
  { 
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return iterator((_Node*)_M_buckets[__n], this);
    return end();
  }

  iterator end() { return iterator((_Node*)0, this); }

  const_iterator begin() const
  {
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return const_iterator((_Node*)_M_buckets[__n], this);
    return end();
  }

  const_iterator end() const { return const_iterator((_Node*)0, this); }

  static bool _STLP_CALL _M_equal (const hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>&,
			const hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>&);

public:

  size_type bucket_count() const { return _M_buckets.size(); }

  size_type max_bucket_count() const
    { return _Stl_prime_type::_M_list[(int)__stl_num_primes - 1]; } 

  size_type elems_in_bucket(size_type __bucket) const
  {
    size_type __result = 0;
    for (_Node* __cur = (_Node*)_M_buckets[__bucket]; __cur; __cur = __cur->_M_next)
      __result += 1;
    return __result;
  }

  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    resize(_M_num_elements._M_data + 1);
    return insert_unique_noresize(__obj);
  }

  iterator insert_equal(const value_type& __obj)
  {
    resize(_M_num_elements._M_data + 1);
    return insert_equal_noresize(__obj);
  }

  pair<iterator, bool> insert_unique_noresize(const value_type& __obj);
  iterator insert_equal_noresize(const value_type& __obj);
 
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l)
  {
    insert_unique(__f, __l, _STLP_ITERATOR_CATEGORY(__f, _InputIterator));
  }

  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l)
  {
    insert_equal(__f, __l, _STLP_ITERATOR_CATEGORY(__f, _InputIterator));
  }

  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l,
                     const input_iterator_tag &)
  {
    for ( ; __f != __l; ++__f)
      insert_unique(*__f);
  }

  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l,
                    const input_iterator_tag &)
  {
    for ( ; __f != __l; ++__f)
      insert_equal(*__f);
  }

  template <class _ForwardIterator>
  void insert_unique(_ForwardIterator __f, _ForwardIterator __l,
                     const forward_iterator_tag &)
  {
    size_type __n = distance(__f, __l);
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  template <class _ForwardIterator>
  void insert_equal(_ForwardIterator __f, _ForwardIterator __l,
                    const forward_iterator_tag &)
  {
    size_type __n = distance(__f, __l);
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }

#else /* _STLP_MEMBER_TEMPLATES */
  void insert_unique(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  void insert_equal(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }

  void insert_unique(const_iterator __f, const_iterator __l)
  {
    size_type __n = distance(__f, __l);
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  void insert_equal(const_iterator __f, const_iterator __l)
  {
    size_type __n = distance(__f, __l);
    resize(_M_num_elements._M_data + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }
#endif /*_STLP_MEMBER_TEMPLATES */

  reference find_or_insert(const value_type& __obj);

private:
# if defined(_STLP_MEMBER_TEMPLATES) && ! defined ( _STLP_NO_EXTENSIONS )  && !(defined(__MRC__)||(defined(__SC__)&&!defined(__DMC_)))
  template <class _KT> 
   _Node* _M_find(const _KT& __key) const
# else
   _Node* _M_find(const key_type& __key) const
# endif
  {
    size_type __n = _M_hash(__key)% _M_buckets.size();
    _Node* __first;
    for ( __first = (_Node*)_M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return __first;
  } 

public:
# if defined(_STLP_MEMBER_TEMPLATES) && ! defined ( _STLP_NO_EXTENSIONS )  && !(defined(__MRC__)||(defined(__SC__)&&!defined(__DMC__)))
  template <class _KT> 
  iterator find(const _KT& __key) 
# else
  iterator find(const key_type& __key) 
# endif
  {
    return iterator(_M_find(__key), this);
  } 

# if defined(_STLP_MEMBER_TEMPLATES) && ! defined ( _STLP_NO_EXTENSIONS )  && !(defined(__MRC__)||(defined(__SC__)&&!defined(__DMC__)))
  template <class _KT> 
  const_iterator find(const _KT& __key) const
# else
  const_iterator find(const key_type& __key) const
# endif
  {
    return const_iterator(_M_find(__key), this);
  } 

  size_type count(const key_type& __key) const
  {
    const size_type __n = _M_bkt_num_key(__key);
    size_type __result = 0;

    for (const _Node* __cur = (_Node*)_M_buckets[__n]; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), __key))
        ++__result;
    return __result;
  }

  pair<iterator, iterator> 
  equal_range(const key_type& __key);

  pair<const_iterator, const_iterator> 
  equal_range(const key_type& __key) const;

  size_type erase(const key_type& __key);
  //   void erase(const iterator& __it); `
  void erase(const const_iterator& __it) ;

  //  void erase(const const_iterator& __first, const const_iterator __last) {
  //     erase((const iterator&)__first, (const iterator&)__last);
  //  }
  void erase(const_iterator __first, const_iterator __last);
  void resize(size_type __num_elements_hint);
  void clear();

public:
  // this is for hash_map::operator[]
  reference _M_insert(const value_type& __obj);

private:

  size_type _M_next_size(size_type __n) const;

  void _M_initialize_buckets(size_type __n)
  {
    const size_type __n_buckets = _M_next_size(__n);
    _M_buckets.reserve(__n_buckets);
    _M_buckets.insert(_M_buckets.end(), __n_buckets, (void*) 0);
    _M_num_elements._M_data = 0;
  }

  size_type _M_bkt_num_key(const key_type& __key) const
  {
    return _M_bkt_num_key(__key, _M_buckets.size());
  }

  size_type _M_bkt_num(const value_type& __obj) const
  {
    return _M_bkt_num_key(_M_get_key(__obj));
  }

  size_type _M_bkt_num_key(const key_type& __key, size_t __n) const
  {
    return _M_hash(__key) % __n;
  }

  size_type _M_bkt_num(const value_type& __obj, size_t __n) const
  {
    return _M_bkt_num_key(_M_get_key(__obj), __n);
  }

  _Node* _M_new_node(const value_type& __obj)
  {
    _Node* __n = _M_num_elements.allocate(1);
    __n->_M_next = 0;
    _STLP_TRY {
      _STLP_STD::_Construct(&__n->_M_val, __obj);
      //      return __n;
    }
    _STLP_UNWIND(_M_num_elements.deallocate(__n, 1));
    return __n;
  }
  
  void _M_delete_node(_Node* __n)
  {
    _STLP_STD::_Destroy(&__n->_M_val);
    _M_num_elements.deallocate(__n, 1);
  }

  void _M_erase_bucket(const size_type __n, _Node* __first, _Node* __last);
  void _M_erase_bucket(const size_type __n, _Node* __last);

  void _M_copy_from(const _Self& __ht);
};

#if 0 // budingc
#define _STLP_TEMPLATE_HEADER template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
#define _STLP_TEMPLATE_CONTAINER hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
#include <stl/_relops_hash_cont.h>
#undef _STLP_TEMPLATE_CONTAINER
#undef _STLP_TEMPLATE_HEADER
#endif // 0

// fbp: these defines are for outline methods definitions.
// needed to definitions to be portable. Should not be used in method bodies.

# if defined ( _STLP_NESTED_TYPE_PARAM_BUG )
#  define __size_type__       size_t
#  define size_type           size_t
#  define value_type      _Val
#  define key_type        _Key
#  define _Node           _Hashtable_node<_Val>
#  define __reference__       _Val&

#  define __iterator__        _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>
#  define __const_iterator__  _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>
# else
#  define __size_type__        _STLP_TYPENAME_ON_RETURN_TYPE hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::size_type
#  define __reference__        _STLP_TYPENAME_ON_RETURN_TYPE  hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::reference
#  define __iterator__         _STLP_TYPENAME_ON_RETURN_TYPE hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::iterator
# endif

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_node<_Val>*
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_skip_to_next() {
  size_t __bucket = _M_ht->_M_bkt_num(_M_cur->_M_val);
  size_t __h_sz;
  __h_sz = this->_M_ht->bucket_count();

  _Node* __i=0;
  while (__i==0 && ++__bucket < __h_sz)
    __i = (_Node*)_M_ht->_M_buckets[__bucket];
  return __i;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
__size_type__
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_next_size(size_type __n) const    { 
  const size_type* __first = (const size_type*)_Stl_prime_type::_M_list;
  const size_type* __last =  (const size_type*)_Stl_prime_type::_M_list + (int)__stl_num_primes;
  const size_type* pos = __lower_bound(__first, __last, __n, __less((size_type*)0), (ptrdiff_t*)0);
  return (pos == __last ? *(__last - 1) : *pos);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
bool 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_equal(
						  const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht1,
						  const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht2)
{
  //  typedef _Hashtable_node<_Val> _Node;
  if (__ht1.bucket_count() != __ht2.bucket_count())
    return false;
  for (size_t __n = 0; __n < __ht1.bucket_count(); ++__n) {
    const _Node* __cur1 = __ht1._M_get_bucket(__n);
    const _Node* __cur2 = __ht2._M_get_bucket(__n);
    for ( ; __cur1 && __cur2 && __cur1->_M_val == __cur2->_M_val;
          __cur1 = __cur1->_M_next, __cur2 = __cur2->_M_next)
      {}
    if (__cur1 || __cur2)
      return false;
  }
  return true;
}  

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> , bool> 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
#ifdef QT_ENABLE_HASH_MAP_REPORT
		::printf("hashtable::insert_unique_noresize, have equal value.\n");
#endif // QT_ENABLE_HASH_MAP_REPORT

      return pair<iterator, bool>(iterator(__cur, this), false);
	}

#ifdef QT_ENABLE_HASH_MAP_REPORT
	int nNum = 0;
	_Node* pFirst = __first;
    for ( ; pFirst; pFirst = pFirst->_M_next)
	{ nNum++; }
	if (nNum >= 100)
		::printf("hashtable::insert_unique_noresize, hash list nNum=%u, bucket_n=%u.\n", 
			nNum, __n);
#endif // QT_ENABLE_HASH_MAP_REPORT

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return pair<iterator, bool>(iterator(__tmp, this), true);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__iterator__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      _Node* __tmp = _M_new_node(__obj);
      __tmp->_M_next = __cur->_M_next;
      __cur->_M_next = __tmp;
      ++_M_num_elements._M_data;
      return iterator(__tmp, this);
    }

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return iterator(__tmp, this);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__reference__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_insert(const value_type& __obj)
{
  resize(_M_num_elements._M_data + 1);

  size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return __tmp->_M_val;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__reference__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::find_or_insert(const value_type& __obj)
{

  _Node* __first = _M_find(_M_get_key(__obj));
  if (__first)
    return __first->_M_val;
  else
    return _M_insert(__obj);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>,
      _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> > 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::equal_range(const key_type& __key)
{
  typedef pair<iterator, iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (_Node* __first = (_Node*)_M_buckets[__n]; __first; __first = __first->_M_next)
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (_Node* __cur = __first->_M_next; __cur; __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(iterator(__first, this), iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(iterator(__first, this),
                     iterator((_Node*)_M_buckets[__m], this));
      return _Pii(iterator(__first, this), end());
    }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>, 
     _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> > 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::equal_range(const key_type& __key) const
{
  typedef pair<const_iterator, const_iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (const _Node* __first = (_Node*)_M_buckets[__n] ;
       __first; 
       __first = __first->_M_next) {
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (const _Node* __cur = __first->_M_next;
           __cur;
           __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(const_iterator(__first, this),
                      const_iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(const_iterator(__first, this),
                      const_iterator((_Node*)_M_buckets[__m], this));
      return _Pii(const_iterator(__first, this), end());
    }
  }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__size_type__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::erase(const key_type& __key)
{
  const size_type __n = _M_bkt_num_key(__key);
  _Node* __first = (_Node*)_M_buckets[__n];
  size_type __erased = 0;

  if (__first) {
    _Node* __cur = __first;
    _Node* __next = __cur->_M_next;
    while (__next) {
      if (_M_equals(_M_get_key(__next->_M_val), __key)) {
        __cur->_M_next = __next->_M_next;
        _M_delete_node(__next);
        __next = __cur->_M_next;
        ++__erased;
        --_M_num_elements._M_data;
      }
      else {
        __cur = __next;
        __next = __cur->_M_next;
      }
    }
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      _M_buckets[__n] = __first->_M_next;
      _M_delete_node(__first);
      ++__erased;
      --_M_num_elements._M_data;
    }
  }
  return __erased;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::erase(const const_iterator& __it)
{
  // const iterator& __it = __REINTERPRET_CAST(const iterator&,_c_it);
  const _Node* __p = __it._M_cur;
  if (__p) {
    const size_type __n = _M_bkt_num(__p->_M_val);
    _Node* __cur = (_Node*)_M_buckets[__n];

    if (__cur == __p) {
      _M_buckets[__n] = __cur->_M_next;
      _M_delete_node(__cur);
      --_M_num_elements._M_data;
    }
    else {
      _Node* __next = __cur->_M_next;
      while (__next) {
        if (__next == __p) {
          __cur->_M_next = __next->_M_next;
          _M_delete_node(__next);
          --_M_num_elements._M_data;
          break;
        }
        else {
          __cur = __next;
          __next = __cur->_M_next;
        }
      }
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::erase(const_iterator _c_first, const_iterator _c_last)
{
  iterator& __first = (iterator&)_c_first;
  iterator& __last = (iterator&)_c_last;
  size_type __f_bucket = __first._M_cur ? 
    _M_bkt_num(__first._M_cur->_M_val) : _M_buckets.size();
  size_type __l_bucket = __last._M_cur ? 
    _M_bkt_num(__last._M_cur->_M_val) : _M_buckets.size();
  if (__first._M_cur == __last._M_cur)
    return;
  else if (__f_bucket == __l_bucket)
    _M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
  else {
    _M_erase_bucket(__f_bucket, __first._M_cur, 0);
    for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
      _M_erase_bucket(__n, 0);
    if (__l_bucket != _M_buckets.size())
      _M_erase_bucket(__l_bucket, __last._M_cur);
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::resize(size_type __num_elements_hint)
{
  const size_type __old_n = _M_buckets.size();
  if (__num_elements_hint > __old_n) {
    const size_type __n = _M_next_size(__num_elements_hint);
    if (__n > __old_n) {
#ifdef QT_ENABLE_HASH_MAP_REPORT
		::printf("hashtable::resize, oldsize=%u, newsize=%u.\n",
			__old_n, __n);
#endif // QT_ENABLE_HASH_MAP_REPORT

      _BucketVector __tmp(__n, (void*)(0),
			  _M_buckets.get_allocator());
      _STLP_TRY {
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = (_Node*)_M_buckets[__bucket];
          while (__first) {
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n);
            _M_buckets[__bucket] = __first->_M_next;
            __first->_M_next = (_Node*)__tmp[__new_bucket];
            __tmp[__new_bucket] = __first;
            __first = (_Node*)_M_buckets[__bucket];          
          }
        }
        _M_buckets.swap(__tmp);
      }
#         ifdef _STLP_USE_EXCEPTIONS
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = ((_Node*)__tmp[__bucket])->_M_next;
            _M_delete_node((_Node*)__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
#         endif /* _STLP_USE_EXCEPTIONS */
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __first, _Node* __last)
{
  _Node* __cur = (_Node*)_M_buckets[__n];
  if (__cur == __first)
    _M_erase_bucket(__n, __last);
  else {
    _Node* __next;
    for (__next = __cur->_M_next; 
         __next != __first; 
         __cur = __next, __next = __cur->_M_next)
      ;
    while (__next != __last) {
      __cur->_M_next = __next->_M_next;
      _M_delete_node(__next);
      __next = __cur->_M_next;
      --_M_num_elements._M_data;
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __last)
{
  _Node* __cur = (_Node*)_M_buckets[__n];
  while (__cur && __cur != __last) {
    _Node* __next = __cur->_M_next;
    _M_delete_node(__cur);
    __cur = __next;
    _M_buckets[__n] = __cur;
    --_M_num_elements._M_data;
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::clear()
{
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = (_Node*)_M_buckets[__i];
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
    _M_buckets[__i] = 0;
  }
  _M_num_elements._M_data = 0;
}

    
template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_copy_from(const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht)
{
  _M_buckets.clear();
  _M_buckets.reserve(__ht._M_buckets.size());
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (void*) 0);
  _STLP_TRY {
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
      const _Node* __cur = (_Node*)__ht._M_buckets[__i];
      if (__cur) {
        _Node* __xcopy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __xcopy;

        for (_Node* __next = __cur->_M_next; 
             __next; 
             __cur = __next, __next = __cur->_M_next) {
          __xcopy->_M_next = _M_new_node(__next->_M_val);
          __xcopy = __xcopy->_M_next;
        }
      }
    }
    _M_num_elements._M_data = __ht._M_num_elements._M_data;
  }
  _STLP_UNWIND(clear());
}

# undef __iterator__ 
# undef const_iterator
# undef __size_type__
# undef __reference__
# undef size_type       
# undef value_type      
# undef key_type        
# undef _Node            
//# undef __stl_num_primes
//# undef hashtable

_STLP_END_NAMESPACE

#endif /* QT_STLP_INTERNAL_HASHTABLE_H */

// Local Variables:
// mode:C++
// End:


