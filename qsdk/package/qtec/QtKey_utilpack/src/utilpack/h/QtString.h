/*------------------------------------------------------*/
/* Connection with Package classes                      */
/*                                                      */
/* QtString.h                                              */
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

#ifndef QTSTRING_H
#define QTSTRING_H

#include <memory>
#include <cassert>

/**********************
 * class flex_string;
 * class CQtCowNewStringStorage;
 */

template <typename E> class CQtTraits;

template <> class CQtTraits<char>
{
public:
	typedef char char_type;
	typedef char E;
	static int compare(const E *d, const E *s, size_t n)
	{return (memcmp(d, s, n)); }
	static size_t length(const E *d)
	{return (strlen(d)); }
	static E * copy(E *d, const E *s, size_t n)
	{return ((E *)memcpy(d, s, n)); }
	static const E * find(const E *d, size_t n,	const E& c)
	{return ((const E *)memchr(d, c, n)); }
	static E * move(E *d, const E *s, size_t n)
	{return ((E *)memmove(d, s, n)); }
	static E * assign(E *d, size_t n, const E& c)
	{return ((E *)memset(d, c, n)); }
	static bool eq (const char_type& c1, const char_type& c2)
    { return (c1 == c2); }
};


////////////////////////////////////////////////////////////////////////////////
// class template SimpleStringStorage
// Reference-NewDelete(Copy On Write) policy
// Allocates memory with new and uses ref count
// Support Only <char> and <w_char>
////////////////////////////////////////////////////////////////////////////////

#define STORAGE_IS_WHOLE(pos, n, str) ((pos) == 0 && (n) >= (str).size())
#define STORAGE_IS_ASSIGN(pos, n, str) STORAGE_IS_WHOLE(pos, n, str)

template <typename E, class A>
class CQtCowNewStringStorage
{
public:
	typedef typename A::size_type size_type;
	typedef E* iterator;
    typedef const E* const_iterator;
    typedef A allocator_type;

	CQtCowNewStringStorage(const CQtCowNewStringStorage& s)
		: m_pData(s.m_pData)
	{
		AddRef(m_pData);
	}

	CQtCowNewStringStorage(const A&)
		: m_pData(const_cast<Data*>(s_pEmptyString))
    {
	}

	CQtCowNewStringStorage(const E* s, size_type len, const A&)
		: m_pData(const_cast<Data*>(s_pEmptyString))
	{
		InitAlloc(len);
		if (m_pData != s_pEmptyString) {
			memcpy((E*)(m_pData + 1), s, len * sizeof(E));
			m_pData->m_uDataLength = len;
			AddTerminateNull();
		}
	}

	~CQtCowNewStringStorage()
    {
		if (m_pData != s_pEmptyString && --m_pData->m_uRefs == 0) {
			operator delete(m_pData);
		}
    }

	iterator begin()             { return (E*)(m_pData + 1); }
    iterator end()               { return (E*)(m_pData + 1) + m_pData->m_uDataLength; }
	const_iterator begin() const { return (E*)(m_pData + 1); }
    const_iterator end()   const { return (E*)(m_pData + 1) + m_pData->m_uDataLength; }

	size_type size()     const { return m_pData->m_uDataLength; }
	size_type max_size() const { return size_t(-1) / sizeof(E) - sizeof(Data) - 1; }
    size_type capacity() const { return m_pData->m_uAllocLength; }

	void resize(size_type n)
	{
		if (n != size())
			InitAlloc(n);
		if (m_pData != s_pEmptyString) {
			m_pData->m_uDataLength = n;
			AddTerminateNull();
		}
	}

    void reserve(size_type res_arg)
    {
		size_type oldSize = size();
		InitAlloc(std::max(res_arg, oldSize));
		if (m_pData != s_pEmptyString)
			m_pData->m_uDataLength = oldSize;
    }

	bool modifier_optimized(size_type pos1, size_type n1, 
		const CQtCowNewStringStorage& another, size_type pos2, size_type n2)
	{
		bool ret = false;
		if (STORAGE_IS_ASSIGN(pos1, n1, *this) && STORAGE_IS_WHOLE(pos2, n2, another)) {
			AddRef(another.m_pData);
			RelRef(m_pData);
			m_pData = another.m_pData;
			ret = true;
		}
		return ret;
	}

	const E* c_str() const { return (E*)(m_pData + 1); }

private:
	struct Data
	{
		unsigned int m_uRefs;
		unsigned int m_uDataLength;
		unsigned int m_uAllocLength;
	};
	static const Data *s_pEmptyString;
	Data *m_pData;

	enum {MIN_SIZE = sizeof (E) <= 32 ? 31 : 7};

	void InitAlloc(size_type aSize)
	{	
		if (m_pData->m_uRefs == 1 && capacity() >= aSize) {

		}
		else {
			// FIXME : calculate capacity size 
		//	size_type uCapacity = aSize ? std::max(aSize, (size_type)16) : 0;
			size_type uCapacity = aSize | MIN_SIZE;
			assert(uCapacity <= max_size());
			Data * pData = const_cast<Data*>(s_pEmptyString);
			
			if (uCapacity) {
				pData = static_cast<Data*>(
					operator new(sizeof(Data) + (uCapacity + 1) * sizeof(E)));
				pData->m_uAllocLength = uCapacity;
				pData->m_uRefs = 1;

				size_type uCopy = std::min(size(), uCapacity);
				if (uCopy)
					memcpy((E*)(pData + 1), begin(), (uCopy + 1) * sizeof(E));
			}
			RelRef(m_pData);
			m_pData = pData;
		}
	}

	void AddTerminateNull()
	{
		assert(m_pData != s_pEmptyString);
		*end() = 0;
	}

	static unsigned int RelRef(Data *&aData)
	{
		assert(aData);
		unsigned int uRet = aData->m_uRefs;

		if (aData != s_pEmptyString) {
			uRet = --aData->m_uRefs;
			if (uRet == 0) {
				delete aData;
				aData = NULL;
			}
		}
		return uRet;
	}

	static unsigned int AddRef(Data *aData)
	{
		assert(aData);
		unsigned int uRet = aData->m_uRefs;
		if (aData != s_pEmptyString) {
			uRet = ++aData->m_uRefs;
		}
		return uRet;
	}
};

static unsigned int s_InitEmptyData[] = {1, 0, 0, 0};

template <typename E, class A> const CQtCowNewStringStorage<E, A>::Data * 
CQtCowNewStringStorage<E, A>::s_pEmptyString = (const CQtCowNewStringStorage<E, A>::Data*)&s_InitEmptyData;


////////////////////////////////////////////////////////////////////////////////
// class template flex_string
// a std::basic_string compatible implementation 
// Uses a Reference-NewDelete policy as default
////////////////////////////////////////////////////////////////////////////////

template <typename E, class T, class A, class Storage >
class flex_string : private Storage
{

//#define Enforce(c, e, m) QTECH_ASSERT_AND_THROW(c, m) 
#define Enforce(expr, excp_type, msg) \
	do { \
		if (!(expr)) { \
			assert(expr); \
		} \
	} \
	while (0)
	
public:
	// types
	typedef T traits_type;
	typedef typename traits_type::char_type value_type;
	typedef A allocator_type;
	typedef typename A::size_type size_type;
	typedef typename A::difference_type difference_type;
    
	typedef typename A::reference reference;
	typedef typename A::const_reference const_reference;
	typedef typename A::pointer pointer;
	typedef typename A::const_pointer const_pointer;
    
	typedef typename Storage::iterator iterator;
	typedef typename Storage::const_iterator const_iterator;

	static const size_type npos;    // = size_type(-1)
    
public:    
	// 21.3.1 construct/copy/destroy
	explicit flex_string(const A& a = A())
		: Storage(a) 
	{ }

	flex_string(const flex_string& str, size_type pos = 0, 
		size_type n = npos, const A& a = A())
		: Storage(a) 
	{
		assign(str, pos, n);
	}

	flex_string(const value_type* s, size_type n = npos, const A& a = A())
		: Storage(s, s ? (n == npos ? traits_type::length(s) : n) : 0, a)
    { }

//	flex_string(size_type n, value_type c, const A& a = A())
//		: Storage(n, c, a)
//	{ }

	template <class InputIterator>
	flex_string(InputIterator begin, InputIterator end, const A& a = A())
		: Storage(a)
	{
		// to do: optimize depending on iterator type
		for (; begin != end; ++begin) *this += E(*begin);
	}

	~flex_string()
	{ }
    
	flex_string& operator=(const flex_string& str) { return assign(str, 0, npos); }
	flex_string& operator=(const value_type* s)	{ return assign(s); }
    flex_string& operator=(value_type c) { return assign(&c, 1); }

	// 21.3.2 iterators:
	iterator begin() { return Storage::begin(); }
	const_iterator begin() const { return Storage::begin(); }
	iterator end() { return Storage::end(); }
	const_iterator end() const { return Storage::end(); }

	// 21.3.3 capacity:
	size_type size() const { return Storage::size(); }
	size_type length() const { return Storage::size(); }
	size_type max_size() const { return Storage::max_size(); }
	size_type capacity() const { return Storage::capacity(); }

	void resize(size_type n) { Storage::resize(n); }

	void reserve(size_type res_arg)
	{
		Enforce(res_arg <= max_size(), (std::length_error*)0, "flex_string::reserve");
		Storage::reserve(res_arg);
	}

	bool empty() const { return Storage::size() == 0; }

	// 21.3.4 element access:
	const_reference operator[](size_type pos) const { return *(begin() + pos); }
	reference operator[](size_type pos) { return *(begin() + pos); }

	const_reference at(size_type n) const
	{
		Enforce(n < size(), (std::out_of_range*)0, "flex_string::at1");
		return (*this)[n];
	}

	reference at(size_type n)
	{
		Enforce(n < size(), (std::out_of_range*)0, "flex_string::at2");
		return (*this)[n];
	}

	// 21.3.5 modifiers:
	flex_string& operator+=(const flex_string& str) { return append(str); }
	flex_string& operator+=(const value_type* s) { return append(s); }
	flex_string& operator+=(value_type c) { return append(&c, 1); }

	flex_string& append(const flex_string& str, size_type pos = 0, size_type n = npos)
	{ return insert(size(), str, pos, n); }

	flex_string& append(const value_type* s, size_type n = npos)
	{ return insert(size(), s, n); }

	flex_string& append(value_type c)
	{ return insert(size(), &c, 1); }
//	flex_string& append(size_type n, value_type c)
//	{ return insert(size(), n, c); }

//	template<class InputIterator>
//	flex_string& append(InputIterator first, InputIterator last)
//	{
//		for (; first != last; ++first) *this += E(*first);
//		return *this;
//	}

	flex_string& assign(const flex_string& str, size_type pos = 0, size_type n = npos)
	{ return replace(0, size(), str, pos, n); }

	flex_string& assign(const value_type* s, size_type n = npos)
	{ return replace(0, size(), s, n); }

//	flex_string& assign(size_type n, value_type c)
//	{ return replace(begin(), end(), n, c); } 

	template<class InputIterator>
	flex_string& assign(InputIterator first, InputIterator last)
	{ return replace(0, size(), first, last - first); }

	flex_string& insert(size_type pos1, const flex_string& str,	size_type pos2 = 0, size_type n = npos)
	{ return replace(pos1, 0, str, pos2, n); }

	flex_string& insert(size_type pos, const value_type* s, size_type n = npos)
	{ return replace(pos, 0, s, n); }

//	flex_string& insert(size_type pos, size_type n, value_type c)
//	{ return replace(pos, 0, n, c); }

//	iterator insert(iterator p, value_type c = value_type()) 
//	{
//		const size_type pos = p - begin();
//		insert(pos, &c, 1);
//		return begin() + pos;
//	}

//	void insert(iterator p, size_type n, value_type c)
//	{ insert(p - begin(), n, c); }

//	template<class InputIterator>
//	void insert(iterator p, InputIterator first, InputIterator last)
//	{ replace(p, p, first, last); }

	flex_string& erase(size_type pos, size_type n)
	{ return replace(pos, n, (const value_type*)NULL, (size_type)0); }

//	iterator erase(iterator position)
//	{
//		const size_type pos(position - begin());
//		erase(pos, 1);
//		return begin() + pos;
//	}

//	iterator erase(iterator first, iterator last)
//	{
//		const size_type pos(first - begin());
//		erase(pos, last - first);
//		return begin() + pos;
//	}

	flex_string& replace(size_type pos1, size_type n1, const flex_string& str,
		size_type pos2 = 0, size_type n2 = npos)
	{
		Enforce(pos1 <= size() && pos2 <= str.size(), (std::out_of_range*)0, "flex_string::replace1");
		n1 = std::min(size() - pos1, n1);
		n2 = std::min(str.size() - pos2, n2);
		
		// FIXME : this == &str
		if (!Storage::modifier_optimized(pos1, n1, str, pos2, n2))
			replace(pos1, n1, &*(str.begin() + pos2), n2);
		return *this;
	}

	flex_string& replace(size_type pos1, size_type n1, const value_type* s,	size_type n2 = npos)
	{
		Enforce(pos1 <= size(), (std::length_error*)0, "flex_string::replace2");
		n1 = std::min(size() - pos1, n1);
		if (n2 == npos || s == NULL)
			n2 = s ? traits_type::length(s) : 0;
		Enforce(n2 + size() <= max_size(), (std::length_error*)0, "flex_string::replace3");

        const size_type newSize = size() -n1 + n2;
		reserve(newSize + end() - (begin() + pos1 + n1));

		iterator first1 = begin() + pos1;
		iterator last1  = first1 + n1;
		const_iterator first2 = s;
//		const_iterator last2 = first2 + n2;

		iterator dest1 = end();
		if (n2 > n1) {
			if (n2 > (size_type)(end() - last1))
				dest1 += n2 - (end() - last1);
			if (end() > last1)
				traits_type::copy(dest1, last1, end() - last1);
		}
		if (n2)
			traits_type::move(first1, first2, n2);
		if (n1 > n2 && end() > last1)
			traits_type::move(first1 + n2, last1, end() - last1);
		else if (n2 > n1 && n2 < (size_type)(end() - last1) && end() > last1)
			traits_type::move(first1 + n2, end(), end() > last1);

        resize(newSize);
		return *this;
    }

//	flex_string& replace(size_type pos, size_type n1, size_type n2, value_type c)
//	{ return replace(pos, n1, flex_string(n2, c)); }

//	flex_string& replace(iterator i1, iterator i2, const flex_string& str)
//	{ return replace(i1, i2, str.c_str(), str.length()); }

//	flex_string& replace(iterator i1, iterator i2, const value_type* s, size_type n)
//	{ return replace(i1 - begin(), i2 - i1, s, n); }

//	flex_string& replace(iterator i1, iterator i2, const value_type* s)
//	{ return replace(i1, i2, s, traits_type::length(s)); }

//	flex_string& replace(iterator i1, iterator i2, size_type n, value_type c)
//	{ return replace(i1 - begin(), i2 - i1, n, c); }

//	template<class InputIterator>
//	flex_string& replace(iterator i1, iterator i2, InputIterator j1, InputIterator j2)
//	{ return replace(i1, i2, flex_string(j1, j2)); }

//	size_type copy(value_type* s, size_type n, size_type pos = 0) const
//	{
//		Enforce(pos <= size(), (std::out_of_range*)0, "");
//		n = std::min(n, size() - pos);
//
//		std::copy(
//			begin() + pos,
//			begin() + pos + n,
//			s);
//		return n;
//	}

//	void swap(flex_string& rhs)
//	{
//		Storage& srhs = rhs;
//		this->Storage::swap(srhs);
//	}

	// 21.3.6 string operations:
	const value_type* c_str() const
	{ return Storage::c_str(); }

	const value_type* data() const
	{ return Storage::c_str(); }

//	allocator_type get_allocator() const
//	{ return Storage::get_allocator(); }

	//Add rfind interface, 2006.4.5 Victor
	size_type rfind(const value_type* s, size_type pos, size_type n)
	{
		if(n > length()) return npos;
		size_type xpos = length() - n;
		if(xpos  > pos)  xpos = pos;
		for (++xpos; xpos-- > 0; )
			if (traits_type::eq (data () [xpos], *s)
				&& traits_type::compare (data () + xpos, s, n) == 0)
				return xpos;
		return npos;
	}
	
	size_type rfind(const flex_string& str, size_type pos = npos) const
	{
		return rfind(str.c_str(), pos, str.length());
	}

	size_type rfind (value_type c, size_type pos = npos) const
	{ return rfind(&c, pos, 1); }
	
	size_type find(const flex_string& str, size_type pos = 0) const
	{ return find(str.c_str(), pos, str.length()); }

	size_type find (const value_type* s, size_type pos = 0, size_type n = npos) const
	{
		Enforce(pos <= size(), (std::out_of_range*)0, "flex_string::find1");
		if (n == npos || s == NULL)
			n = s ? traits_type::length(s) : 0;
		size_type nm = size() - pos;
		if (n && n <= nm) {
			const value_type *first = c_str() + pos, *last;
			nm -= n - 1;
			while (last = traits_type::find(first, nm, *s)) {
				if (n == 1 || !traits_type::compare(last, s, n))
					return last - c_str();
				nm -= last - first + 1;
				first = last + 1;
			}
		}
		return npos;
	}

	size_type find (value_type c, size_type pos = 0) const
	{ return find(&c, pos, 1); }

	flex_string substr(size_type pos = 0, size_type n = npos) const
	{
		Enforce(pos <= size(), (std::out_of_range*)0, "flex_string::substr");
		return flex_string(c_str() + pos, std::min(n, size() - pos));
	}

	int compare(const flex_string& str) const
	{ return compare(0, size(), str.c_str(), str.size()); }

	int compare(size_type pos1, size_type n1, const flex_string& str) const
	{ return compare(pos1, n1, str.c_str(), str.size()); }

	int compare(size_type pos1, size_type n1,const value_type* s, size_type n2 = npos) const
	{
		Enforce(pos1 <= size(), (std::out_of_range*)0, "flex_string::compare1");
		if (n2 == npos || s == NULL)
			n2 = s ? traits_type::length(s) : 0;
		n1 = std::min(size() - pos1, n1);
		int result = traits_type::compare(c_str() + pos1, s, std::min(n1, n2));
		return (result != 0) ? result : (n1 - n2);
	}

	int compare(size_type pos1, size_type n1,
		const flex_string& str, size_type pos2, size_type n2) const
	{
		Enforce(pos2 <= str.size(), (std::out_of_range*)0, "flex_string::compare2");
		return compare(pos1, n1, str.c_str() + pos2, std::min(n2, str.size() - pos2));
	}

	int compare(const value_type* s) const
	{ return compare(0, size(), s, npos); }

#undef Enforce
};

// non-member functions
template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, 
    const flex_string<E, T, A, S>& rhs)
{
	flex_string<E, T, A, S> result;
	result.reserve(lhs.size() + rhs.size());
	result.append(lhs);
	result.append(rhs);
	return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(
	const flex_string<E, T, A, S>::value_type* lhs, 
    const flex_string<E, T, A, S>& rhs)
{
	return rhs + lhs;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(
    flex_string<E, T, A, S>::value_type lhs, 
    const flex_string<E, T, A, S>& rhs)
{
	return rhs + lhs;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(
	const flex_string<E, T, A, S>& lhs, 
	const flex_string<E, T, A, S>::value_type* rhs)
{
	flex_string<E, T, A, S> result;
	flex_string<E, T, A, S>::size_type len = rhs ? T::length(rhs) : 0;
	result.reserve(lhs.size() + len);
	result.append(lhs);
	result.append(rhs, len);
	return result;
}

template <typename E, class T, class A, class S>
flex_string<E, T, A, S> operator+(const flex_string<E, T, A, S>& lhs, flex_string<E, T, A, S>::value_type rhs)
{
	flex_string<E, T, A, S> result;
	result.reserve(lhs.size() + 1);
	result.append(lhs);
	result.append(rhs);
	return result;
}

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return lhs.compare(rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return rhs == lhs; }

template <typename E, class T, class A, class S>
bool operator==(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return lhs.compare(rhs) == 0; }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator!=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs == rhs); }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return lhs.compare(rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return lhs.compare(rhs) < 0; }

template <typename E, class T, class A, class S>
bool operator<(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return rhs.compare(lhs) > 0; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator>(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return rhs < lhs; }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator<=(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return !(rhs < lhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>& lhs, const flex_string<E, T, A, S>::value_type* rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
bool operator>=(const flex_string<E, T, A, S>::value_type* lhs, const flex_string<E, T, A, S>& rhs)
{ return !(lhs < rhs); }

template <typename E, class T, class A, class S>
const flex_string<E, T, A, S>::size_type
flex_string<E, T, A, S>::npos = 
(typename flex_string<E, T, A, S>::size_type)(-1);


#endif // !QTSTRING_H
