
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


#include "QtBase.h"
#ifndef QT_SOLARIS
#include "QtHashMapT.h"
_STLP_BEGIN_NAMESPACE
# define __PRIME_LIST_BODY { \
	53ul,         97ul,         193ul,       389ul,       769ul,      \
	1543ul,       3079ul,       6151ul,      12289ul,     24593ul,    \
	49157ul,      98317ul,      196613ul,    393241ul,    786433ul,   \
	1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul, \
	50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,\
	1610612741ul, 3221225473ul, 4294967291ul  \
}
  const size_t _Stl_prime_type::_M_list[__stl_num_primes] = __PRIME_LIST_BODY;
# undef __PRIME_LIST_BODY
_STLP_END_NAMESPACE
#endif

