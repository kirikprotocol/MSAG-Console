/* ************************************************************************* *
 * Helper templates representing CHOICE classes.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_CHOICE_OF_HPP
#ident "@(#)$Id$"
#define __EYELINE_UTIL_CHOICE_OF_HPP

#include "eyeline/util/UnionT.hpp"

namespace eyeline {
namespace util {

/* ************************************************************************* *
 * Helper template for CHOICE of two types ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2
>
class ChoiceOf2_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public UnionAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf2_T;

    Alternative_T(UnionStorageAC & use_store)
      : UnionAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf2_T;

    ConstAlternative_T(const UnionStorageAC & use_store)
      : UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
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

  void clear() { _union.clear(); }

protected:
  UnionStorageOf2_T<_TArg1, _TArg2> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of two types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2
>
class ChoiceOfBased2_T : public ChoiceOf2_T<_TArg1, _TArg2> {
public:
  ChoiceOfBased2_T() : ChoiceOf2_T<_TArg1, _TArg2>()
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
>
class ChoiceOf3_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public UnionAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf3_T;

    Alternative_T(UnionStorageAC & use_store)
      : UnionAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf3_T;

    ConstAlternative_T(const UnionStorageAC & use_store)
      : UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
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

  void clear() { _union.clear(); }

protected:
  UnionStorageOf3_T<_TArg1, _TArg2, _TArg3> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }

  Alternative_T<_TArg3, 2>      alternative2()        { return Alternative_T<_TArg3, 2>(_union); }
  ConstAlternative_T<_TArg3, 2> alternative2() const  { return ConstAlternative_T<_TArg3, 2>(_union); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of three types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3
>
class ChoiceOfBased3_T : public ChoiceOf3_T<_TArg1, _TArg2, _TArg3> {
public:
  ChoiceOfBased3_T() : ChoiceOf3_T<_TArg1, _TArg2, _TArg3>()
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
>
class ChoiceOf4_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public UnionAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf4_T;

    Alternative_T(UnionStorageAC & use_store)
      : UnionAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf4_T;

    ConstAlternative_T(const UnionStorageAC & use_store)
      : UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
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

  void clear() { _union.clear(); }

protected:
  UnionStorageOf4_T<_TArg1, _TArg2, _TArg3, _TArg4> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }

  Alternative_T<_TArg3, 2>      alternative2()        { return Alternative_T<_TArg3, 2>(_union); }
  ConstAlternative_T<_TArg3, 2> alternative2() const  { return ConstAlternative_T<_TArg3, 2>(_union); }

  Alternative_T<_TArg4, 3>      alternative3()        { return Alternative_T<_TArg4, 3>(_union); }
  ConstAlternative_T<_TArg4, 3> alternative3() const  { return ConstAlternative_T<_TArg4, 3>(_union); }
};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of four types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class ChoiceOfBased4_T : public ChoiceOf4_T<_TArg1, _TArg2, _TArg3, _TArg4> {
public:
  ChoiceOfBased4_T() : ChoiceOf4_T<_TArg1, _TArg2, _TArg3, _TArg4>()
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
>
class ChoiceOf5_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public UnionAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf5_T;

    Alternative_T(UnionStorageAC & use_store)
      : UnionAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOf5_T;

    ConstAlternative_T(const UnionStorageAC & use_store)
      : UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
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

  void clear() { _union.clear(); }

protected:
  UnionStorageOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }

  Alternative_T<_TArg3, 2>      alternative2()        { return Alternative_T<_TArg3, 2>(_union); }
  ConstAlternative_T<_TArg3, 2> alternative2() const  { return ConstAlternative_T<_TArg3, 2>(_union); }

  Alternative_T<_TArg4, 3>      alternative3()        { return Alternative_T<_TArg4, 3>(_union); }
  ConstAlternative_T<_TArg4, 3> alternative3() const  { return ConstAlternative_T<_TArg4, 3>(_union); }

  Alternative_T<_TArg5, 4>      alternative4()        { return Alternative_T<_TArg5, 4>(_union); }
  ConstAlternative_T<_TArg5, 4> alternative4() const  { return ConstAlternative_T<_TArg5, 4>(_union); }

};

/* ------------------------------------------------------------------------- *
 * Helper template for CHOICE of five types, which have same base type ...
 * ------------------------------------------------------------------------- */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
>
class ChoiceOfBased5_T : public ChoiceOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5> {
public:
  ChoiceOfBased5_T() : ChoiceOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5>()
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

} //util
} //eyeline

#endif /* __EYELINE_UTIL_CHOICE_OF_HPP */

