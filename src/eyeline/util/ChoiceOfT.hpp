/* ************************************************************************* *
 * Helper templates representing CHOICE classes.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_CHOICE_OF_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_UTIL_CHOICE_OF_HPP

#include "eyeline/util/UnionT.hpp"

namespace eyeline {
namespace util {

/* ************************************************************************* *
 * Helper template for CHOICE of two types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOf2_T {
public:
  template < class _TArg >
  class Alternative_T : public _AlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf2_T;

    Alternative_T(UnionStorageAC & use_store, uint16_t use_altIdx)
      : _AlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg >
  class ConstAlternative_T : public _ConstAlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf2_T;

    ConstAlternative_T(const UnionStorageAC & use_store, uint16_t use_altIdx)
      : _ConstAlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOf2_T()
  { }
  ~ChoiceOf2_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  bool empty(void) const { return _union.empty(); }

  void clear() { _union.clear(); }

protected:
  UnionStorageOf2_T<_TArg1, _TArg2> _union;

  Alternative_T<_TArg1>      alternative0()        { return Alternative_T<_TArg1>(_union, 0); }
  ConstAlternative_T<_TArg1> alternative0() const  { return ConstAlternative_T<_TArg1>(_union, 0); }

  Alternative_T<_TArg2>      alternative1()        { return Alternative_T<_TArg2>(_union, 1); }
  ConstAlternative_T<_TArg2> alternative1() const  { return ConstAlternative_T<_TArg2>(_union, 1); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of two types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOfBased2_T
  : public ChoiceOf2_T<_TArg1, _TArg2, _AlternativeTArg, _ConstAlternativeTArg> {
public:
  ChoiceOfBased2_T()
    : ChoiceOf2_T<_TArg1, _TArg2, _AlternativeTArg, _ConstAlternativeTArg>()
  { }
  ~ChoiceOfBased2_T()
  { }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
};

/* ************************************************************************* *
 * Helper template class for CHOICE of three types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOf3_T {
public:
  template < class _TArg >
  class Alternative_T : public _AlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf3_T;

    Alternative_T(UnionStorageAC & use_store, uint16_t use_altIdx)
      : _AlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg >
  class ConstAlternative_T : public _ConstAlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf3_T;

    ConstAlternative_T(const UnionStorageAC & use_store, uint16_t use_altIdx)
      : _ConstAlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOf3_T()
  { }
  ~ChoiceOf3_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  bool empty(void) const { return _union.empty(); }

  void clear() { _union.clear(); }

protected:
  UnionStorageOf3_T<_TArg1, _TArg2, _TArg3> _union;

  Alternative_T<_TArg1>      alternative0()        { return Alternative_T<_TArg1>(_union, 0); }
  ConstAlternative_T<_TArg1> alternative0() const  { return ConstAlternative_T<_TArg1>(_union, 0); }

  Alternative_T<_TArg2>      alternative1()        { return Alternative_T<_TArg2>(_union, 1); }
  ConstAlternative_T<_TArg2> alternative1() const  { return ConstAlternative_T<_TArg2>(_union, 1); }

  Alternative_T<_TArg3>      alternative2()        { return Alternative_T<_TArg3>(_union, 2); }
  ConstAlternative_T<_TArg3> alternative2() const  { return ConstAlternative_T<_TArg3>(_union, 2); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of three types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOfBased3_T
  : public ChoiceOf3_T<_TArg1, _TArg2, _TArg3, _AlternativeTArg, _ConstAlternativeTArg> {
public:
  ChoiceOfBased3_T()
    : ChoiceOf3_T<_TArg1, _TArg2, _TArg3, _AlternativeTArg, _ConstAlternativeTArg>()
  { }
  ~ChoiceOfBased3_T()
  { }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
};

/* ************************************************************************* *
 * Helper template class for CHOICE of four types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOf4_T {
public:
  template < class _TArg >
  class Alternative_T : public _AlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf4_T;

    Alternative_T(UnionStorageAC & use_store, uint16_t use_altIdx)
      : _AlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg >
  class ConstAlternative_T : public _ConstAlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf4_T;

    ConstAlternative_T(const UnionStorageAC & use_store, uint16_t use_altIdx)
      : _ConstAlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOf4_T()
  { }
  ~ChoiceOf4_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  bool empty(void) const { return _union.empty(); }

  void clear() { _union.clear(); }

protected:
  UnionStorageOf4_T<_TArg1, _TArg2, _TArg3, _TArg4> _union;

  Alternative_T<_TArg1>      alternative0()        { return Alternative_T<_TArg1>(_union, 0); }
  ConstAlternative_T<_TArg1> alternative0() const  { return ConstAlternative_T<_TArg1>(_union, 0); }

  Alternative_T<_TArg2>      alternative1()        { return Alternative_T<_TArg2>(_union, 1); }
  ConstAlternative_T<_TArg2> alternative1() const  { return ConstAlternative_T<_TArg2>(_union, 1); }

  Alternative_T<_TArg3>      alternative2()        { return Alternative_T<_TArg3>(_union, 2); }
  ConstAlternative_T<_TArg3> alternative2() const  { return ConstAlternative_T<_TArg3>(_union, 2); }

  Alternative_T<_TArg4>      alternative3()        { return Alternative_T<_TArg4>(_union, 3); }
  ConstAlternative_T<_TArg4> alternative3() const  { return ConstAlternative_T<_TArg4>(_union, 3); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of four types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOfBased4_T
  : public ChoiceOf4_T<_TArg1, _TArg2, _TArg3, _TArg4, _AlternativeTArg, _ConstAlternativeTArg> {
public:
  ChoiceOfBased4_T()
    : ChoiceOf4_T<_TArg1, _TArg2, _TArg3, _TArg4, _AlternativeTArg, _ConstAlternativeTArg>()
  { }
  ~ChoiceOfBased4_T()
  { }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
};

/* ************************************************************************* *
 * Helper template class for CHOICE of five types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOf5_T {
public:
  template < class _TArg >
  class Alternative_T : public _AlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf5_T;

    Alternative_T(UnionStorageAC & use_store, uint16_t use_altIdx)
      : _AlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg >
  class ConstAlternative_T : public _ConstAlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf5_T;

    ConstAlternative_T(const UnionStorageAC & use_store, uint16_t use_altIdx)
      : _ConstAlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOf5_T()
  { }
  ~ChoiceOf5_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  bool empty(void) const { return _union.empty(); }

  void clear() { _union.clear(); }

protected:
  UnionStorageOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5> _union;

  Alternative_T<_TArg1>      alternative0()        { return Alternative_T<_TArg1>(_union, 0); }
  ConstAlternative_T<_TArg1> alternative0() const  { return ConstAlternative_T<_TArg1>(_union, 0); }

  Alternative_T<_TArg2>      alternative1()        { return Alternative_T<_TArg2>(_union, 1); }
  ConstAlternative_T<_TArg2> alternative1() const  { return ConstAlternative_T<_TArg2>(_union, 1); }

  Alternative_T<_TArg3>      alternative2()        { return Alternative_T<_TArg3>(_union, 2); }
  ConstAlternative_T<_TArg3> alternative2() const  { return ConstAlternative_T<_TArg3>(_union, 2); }

  Alternative_T<_TArg4>      alternative3()        { return Alternative_T<_TArg4>(_union, 3); }
  ConstAlternative_T<_TArg4> alternative3() const  { return ConstAlternative_T<_TArg4>(_union, 3); }

  Alternative_T<_TArg5>      alternative4()        { return Alternative_T<_TArg5>(_union, 4); }
  ConstAlternative_T<_TArg5> alternative4() const  { return ConstAlternative_T<_TArg5>(_union, 4); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of five types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOfBased5_T
  : public ChoiceOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5, _AlternativeTArg, _ConstAlternativeTArg> {
public:
  ChoiceOfBased5_T()
    : ChoiceOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5, _AlternativeTArg, _ConstAlternativeTArg>()
  { }
  ~ChoiceOfBased5_T()
  { }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
};


/* ************************************************************************* *
 * Helper template class for CHOICE of six types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5, class _TArg6
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOf6_T {
public:
  template < class _TArg >
  class Alternative_T : public _AlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf6_T;

    Alternative_T(UnionStorageAC & use_store, uint16_t use_altIdx)
      : _AlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg >
  class ConstAlternative_T : public _ConstAlternativeTArg<_TArg> {
  protected:
    friend class ChoiceOf6_T;

    ConstAlternative_T(const UnionStorageAC & use_store, uint16_t use_altIdx)
      : _ConstAlternativeTArg<_TArg>(use_store, use_altIdx)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOf6_T()
  { }
  ~ChoiceOf6_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  bool empty(void) const { return _union.empty(); }

  void clear() { _union.clear(); }

protected:
  UnionStorageOf6_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5, _TArg6> _union;

  Alternative_T<_TArg1>      alternative0()        { return Alternative_T<_TArg1>(_union, 0); }
  ConstAlternative_T<_TArg1> alternative0() const  { return ConstAlternative_T<_TArg1>(_union, 0); }

  Alternative_T<_TArg2>      alternative1()        { return Alternative_T<_TArg2>(_union, 1); }
  ConstAlternative_T<_TArg2> alternative1() const  { return ConstAlternative_T<_TArg2>(_union, 1); }

  Alternative_T<_TArg3>      alternative2()        { return Alternative_T<_TArg3>(_union, 2); }
  ConstAlternative_T<_TArg3> alternative2() const  { return ConstAlternative_T<_TArg3>(_union, 2); }

  Alternative_T<_TArg4>      alternative3()        { return Alternative_T<_TArg4>(_union, 3); }
  ConstAlternative_T<_TArg4> alternative3() const  { return ConstAlternative_T<_TArg4>(_union, 3); }

  Alternative_T<_TArg5>      alternative4()        { return Alternative_T<_TArg5>(_union, 4); }
  ConstAlternative_T<_TArg5> alternative4() const  { return ConstAlternative_T<_TArg5>(_union, 4); }

  Alternative_T<_TArg6>      alternative5()        { return Alternative_T<_TArg6>(_union, 5); }
  ConstAlternative_T<_TArg6> alternative5() const  { return ConstAlternative_T<_TArg6>(_union, 5); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of six types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5, class _TArg6
  , template <class> class _AlternativeTArg = UnionAlternative_T
  , template <class> class _ConstAlternativeTArg = UnionConstAlternative_T
>
class ChoiceOfBased6_T
  : public ChoiceOf6_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5, _TArg6, _AlternativeTArg, _ConstAlternativeTArg> {
public:
  ChoiceOfBased6_T()
    : ChoiceOf6_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5, _TArg6, _AlternativeTArg, _ConstAlternativeTArg>()
  { }
  ~ChoiceOfBased6_T()
  { }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
};

} //util
} //eyeline

#endif /* __EYELINE_UTIL_CHOICE_OF_HPP */

