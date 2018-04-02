
#include "QtBase.h"
#include "QtAtomicOperationT.h"

// mainly copied from ace/Atomic_Op.cpp
#define ACE_reinterpret_cast(TYPE, EXPR)  reinterpret_cast<TYPE> (EXPR)
#define ACE_NOTSUP_RETURN(FAILVALUE) do { /*errno = ENOTSUP ;*/ return FAILVALUE; } while (0)
#define ACE_HAS_PENTIUM QT_HAS_PENTIUM
#if defined (ghs) || defined (__GNUC__) || defined (__hpux) || defined (__sgi) || defined (__DECCXX) || defined (__KCC) || defined (__rational__) || defined (__USLC__) || defined (ACE_RM544)
// Some compilers complain about "statement with no effect" with (a).
// This eliminates the warnings, and no code is generated for the null
// conditional statement.  NOTE: that may only be true if -O is enabled,
// such as with GreenHills (ghs) 1.8.8.
  #define ACE_UNUSED_ARG(a) do {/* null */} while (&a == 0)
#else /* ghs || __GNUC__ || ..... */
  #define ACE_UNUSED_ARG(a) (a)
#endif /* ghs || __GNUC__ || ..... */


#ifdef QT_HAS_BUILTIN_ATOMIC_OP

long (*CQtAtomicOperationT<CQtMutexThread>::increment_fn_) (volatile long *) = 0;
long (*CQtAtomicOperationT<CQtMutexThread>::decrement_fn_) (volatile long *) = 0;
long (*CQtAtomicOperationT<CQtMutexThread>::exchange_fn_) (volatile long *, long) = 0;
long (*CQtAtomicOperationT<CQtMutexThread>::exchange_add_fn_) (volatile long *, long) = 0;

static long num_processors (void)
{
#if defined (QT_WIN32)
	SYSTEM_INFO sys_info;
	::GetSystemInfo (&sys_info);
	return sys_info.dwNumberOfProcessors;
#elif defined (QT_UNIX)
	return ::sysconf (_SC_NPROCESSORS_CONF);
#elif defined (QT_MACOS)
	return MPProcessorsScheduled();
#else	
	#error ERROR: Do not supports other platform!
	return -1;
#endif
}

void CQtAtomicOperationT<CQtMutexThread>::init_functions (void)
{
  if (num_processors () == 1)
    {
      increment_fn_ = single_cpu_increment;
      decrement_fn_ = single_cpu_decrement;
      exchange_fn_ = single_cpu_exchange;
      exchange_add_fn_ = single_cpu_exchange_add;
    }
  else
    {
      increment_fn_ = multi_cpu_increment;
      decrement_fn_ = multi_cpu_decrement;
      exchange_fn_ = multi_cpu_exchange;
      exchange_add_fn_ = multi_cpu_exchange_add;
    }
}

#if defined (_MSC_VER)
// Disable "no return value" warning, as we will be putting
// the return values directly into the EAX register.
#pragma warning (push)
#pragma warning (disable: 4035)
#endif /* _MSC_VER */

long
CQtAtomicOperationT<CQtMutexThread>::single_cpu_increment (volatile long *value)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  long tmp = 1;
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "xadd %0, (%1)" : "+r"(tmp) : "r"(addr) );
  return tmp + 1;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::single_cpu_decrement (volatile long *value)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  long tmp = -1;
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "xadd %0, (%1)" : "+r"(tmp) : "r"(addr) );
  return tmp - 1;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::single_cpu_exchange (
  volatile long *value,
  long rhs)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "xchg %0, (%1)" : "+r"(rhs) : "r"(addr) );
  return rhs;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::single_cpu_exchange_add (volatile long *value,
                                                                long rhs)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "xadd %0, (%1)" : "+r"(rhs) : "r"(addr) );
  return rhs;
#elif defined (WIN32) && !defined (ACE_HAS_INTERLOCKED_EXCHANGEADD)
# if defined (_MSC_VER)
  __asm
    {
      mov eax, rhs
      mov edx, value
      xadd [edx], eax
    }
  // Return value is already in EAX register.
# elif defined (__BORLANDC__)
  _EAX = rhs;
  _EDX = ACE_reinterpret_cast (unsigned long, value);
  __emit__(0x0F, 0xC1, 0x02); // xadd [edx], eax
  // Return value is already in EAX register.
# else /* _MSC_VER */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
# endif /* _MSC_VER */
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::multi_cpu_increment (volatile long *value)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  long tmp = 1;
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "lock ; xadd %0, (%1)" : "+r"(tmp) : "r"(addr) );
  return tmp + 1;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::multi_cpu_decrement (volatile long *value)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  long tmp = -1;
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "lock ; xadd %0, (%1)" : "+r"(tmp) : "r"(addr) );
  return tmp - 1;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::multi_cpu_exchange (
  volatile long *value,
  long rhs)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  // The XCHG instruction automatically follows LOCK semantics
  asm( "xchg %0, (%1)" : "+r"(rhs) : "r"(addr) );
  return rhs;
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

long
CQtAtomicOperationT<CQtMutexThread>::multi_cpu_exchange_add (volatile long *value,
                                                               long rhs)
{
#if defined (__GNUC__) && defined (ACE_HAS_PENTIUM)
  unsigned long addr = ACE_reinterpret_cast (unsigned long, value);
  asm( "lock ; xadd %0, (%1)" : "+r"(rhs) : "r"(addr) );
  return rhs;
#elif defined (WIN32) && !defined (ACE_HAS_INTERLOCKED_EXCHANGEADD)
# if defined (_MSC_VER)
  __asm
    {
      mov eax, rhs
      mov edx, value
      lock xadd [edx], eax
    }
  // Return value is already in EAX register.
# elif defined (__BORLANDC__)
  _EAX = rhs;
  _EDX = ACE_reinterpret_cast (unsigned long, value);
  __emit__(0xF0, 0x0F, 0xC1, 0x02); // lock xadd [edx], eax
  // Return value is already in EAX register.
# else /* _MSC_VER */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
# endif /* _MSC_VER */
#else /* __GNUC__ && ACE_HAS_PENTIUM */
  ACE_UNUSED_ARG (value);
  ACE_UNUSED_ARG (rhs);
  ACE_NOTSUP_RETURN (-1);
#endif /* __GNUC__ && ACE_HAS_PENTIUM */
}

#if defined (_MSC_VER)
#pragma warning (pop)
#endif /* _MSC_VER */

#endif // QT_HAS_BUILTIN_ATOMIC_OP
