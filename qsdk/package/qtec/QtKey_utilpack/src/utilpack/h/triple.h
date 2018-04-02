/*------------------------------------------------------*/
/* the triple case definition	         		*/
/*                                                      */
/* triple.h                                             */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin     (zhubin@qtec.cn)                       */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*------------------------------------------------------*/

#ifndef TRIPLE_INC_
#define TRIPLE_INC_

template <typename T1, typename T2, typename T3>
class triple
{
public:

    typedef T1 first_type;
    typedef T2 second_type;
    typedef T3 third_type;

    triple (const T1 &t1 = T1(),
          const T2 &t2 = T2(), 
          const T3 &t3 = T3()):first_(t1), second_(t2), third_(t3)
    {   }

	 triple(const triple<T1, T2, T3> &rhs):first_(rhs.first()), second_(rhs.second()), third_(rhs.third())
	 {	 }

	 triple& operator=(const triple<T1, T2, T3> &rhs)
	 {
		 this->first_ = *(T1 *)(&rhs.first());
		 this->second_ = *(T2 *)(&rhs.second());
		 this->third_ = rhs.third();
		 return *this;
	 }

    const first_type &first (void) const
    {
		 return first_;
    }
    first_type &first (void)
    {
		 return first_;
    }

    void first(const first_type &t1) 
    {
        first_ = t1;
    }

    const second_type &second (void) const
    {
		 return second_;
    }
    second_type &second (void)
    {
		 return second_;
    }

    void second(const second_type &t2)
    {
        second_ = t2;
    }

    const third_type &third (void) const
    {
		 return third_;
    }
    third_type &third (void)
    {
		 return third_;
    }

    void third(const third_type &t3)
    {
        third_ = t3;
    }

    int operator== (const triple<T1, T2, T3> &rhs) const
    {
        return (rhs.first() == first() && rhs.second() == second() && rhs.third() == third());
    }

    int operator != (const triple<T1, T2, T3> &rhs) const
    {
        return (rhs.first() != first() || rhs.second() != second() || rhs.third() != third());
    }
#if defined(__GNUC__)
    template <typename _T1, typename _T2, typename _T3>
    friend bool operator<(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_);
    template <typename _T1, typename _T2, typename _T3>
    friend bool operator>(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_);
    template <typename _T1, typename _T2, typename _T3>
    friend bool operator<=(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_);
    template <typename _T1, typename _T2, typename _T3>
    friend bool operator>=(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_);
#else
    friend bool operator<(const triple<T1, T2, T3>& lhs_, const triple<T1, T2, T3>& rhs_);
    friend bool operator>(const triple<T1, T2, T3>& lhs_, const triple<T1, T2, T3>& rhs_);
    friend bool operator<=(const triple<T1, T2, T3>& lhs_, const triple<T1, T2, T3>& rhs_);
    friend bool operator>=(const triple<T1, T2, T3>& lhs_, const triple<T1, T2, T3>& rhs_);
#endif

private:
    first_type first_;
    second_type second_;
    third_type third_;
};


template <typename _T1, typename _T2, typename _T3>
inline bool operator<(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_)
{ 
  return lhs_.first_ < rhs_.first_ || 
         (lhs_.first_ == rhs_.first_ && lhs_.second_ < rhs_.second_) ||
         (lhs_.first_ == rhs_.first_ && lhs_.second_ == rhs_.second_ && lhs_.third_ < rhs_.third_); 
}

template <typename _T1, typename _T2, typename _T3>
inline bool operator>(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_) {
  return !(rhs_ <= lhs_);
}

template <typename _T1, typename _T2, typename _T3>
inline bool operator<=(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_) {
  return lhs_.first_ <= rhs_.first_ || 
         (lhs_.first_ == rhs_.first_ && lhs_.second_ <= rhs_.second_) ||
         (lhs_.first_ == rhs_.first_ && lhs_.second_ == rhs_.second_ && lhs_.third_ <= rhs_.third_); 
}

template <typename _T1, typename _T2, typename _T3>
inline bool operator>=(const triple<_T1, _T2, _T3>& lhs_, const triple<_T1, _T2, _T3>& rhs_) {
  return !(lhs_ < rhs_);
}

template <typename _T1, typename _T2, typename _T3>
inline triple<_T1, _T2, _T3> make_triple(const _T1& first_val, const _T2& second_val, const _T3 &third_val)
{
  return triple<_T1, _T2, _T3>(first_val, second_val, third_val);
}

#endif

