/* ************************************************************************* *
 * BER [En/de]coder: helper template class for creation of union of 
 *                   Type[En/De]coders in same preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_CODERS_CHOICE_HPP
#ident "@(#)$Id$"
#define __ASN1_BER_CODERS_CHOICE_HPP

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

#include "eyeline/util/UnionT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  //must have tagged & untagged type constructors, copying constructor
  class _CoderOfTArg /*: public TypeValueEncoder_T<_TArg>(TransferSyntax::Rule_e use_rule)*/ 
, uint16_t _altIdxArg
> 
class CoderAlternative_T : public util::UnionAlternative_T<_CoderOfTArg, _altIdxArg> {
public:
  CoderAlternative_T(util::UnionStorageAC & use_store)
    : util::UnionAlternative_T<_CoderOfTArg, _altIdxArg>(use_store)
  { }
  ~CoderAlternative_T()
  { }

  //
  _CoderOfTArg  & init(TransferSyntax::Rule_e use_rule)
  {
    this->_store.clear();
    this->_store.setIdx(_altIdxArg);
    return *(new (this->_store.get())_CoderOfTArg(use_rule));
  }
  //
  _CoderOfTArg  & init(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule)
  {
    this->_store.clear();
    this->_store.setIdx(_altIdxArg);
    return *(new (this->_store.get())_CoderOfTArg(use_tag, tag_env, use_rule));
  }
};

/* ************************************************************************* *
 * Helper template for union of two encoders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2
>
class ChoiceOfCoders2_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public CoderAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders2_T;

    Alternative_T(util::UnionStorageAC & use_store)
      : CoderAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public util::UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders2_T;

    ConstAlternative_T(const util::UnionStorageAC & use_store)
      : util::UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOfCoders2_T()
  { }
  ~ChoiceOfCoders2_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  void clear() { _union.clear(); }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }

protected:
  util::UnionStorageOf2_T<_TArg1, _TArg2> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }
};

/* ************************************************************************* *
 * Helper template for union of three encoders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3
>
class ChoiceOfCoders3_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public CoderAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders3_T;

    Alternative_T(util::UnionStorageAC & use_store)
      : CoderAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public util::UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders3_T;

    ConstAlternative_T(const util::UnionStorageAC & use_store)
      : util::UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOfCoders3_T()
  { }
  ~ChoiceOfCoders3_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  void clear() { _union.clear(); }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }

protected:
  util::UnionStorageOf3_T<_TArg1, _TArg2, _TArg3> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }

  Alternative_T<_TArg3, 2>      alternative2()        { return Alternative_T<_TArg3, 2>(_union); }
  ConstAlternative_T<_TArg3, 2> alternative2() const  { return ConstAlternative_T<_TArg3, 2>(_union); }
};

/* ************************************************************************* *
 * Helper template for union of four encoders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class ChoiceOfCoders4_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public CoderAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders4_T;

    Alternative_T(util::UnionStorageAC & use_store)
      : CoderAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public util::UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders4_T;

    ConstAlternative_T(const util::UnionStorageAC & use_store)
      : util::UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOfCoders4_T()
  { }
  ~ChoiceOfCoders4_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  void clear() { _union.clear(); }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }

protected:
  util::UnionStorageOf4_T<_TArg1, _TArg2, _TArg3, _TArg4> _union;

  Alternative_T<_TArg1, 0>      alternative0()        { return Alternative_T<_TArg1, 0>(_union); }
  ConstAlternative_T<_TArg1, 0> alternative0() const  { return ConstAlternative_T<_TArg1, 0>(_union); }

  Alternative_T<_TArg2, 1>      alternative1()        { return Alternative_T<_TArg2, 1>(_union); }
  ConstAlternative_T<_TArg2, 1> alternative1() const  { return ConstAlternative_T<_TArg2, 1>(_union); }

  Alternative_T<_TArg3, 2>      alternative2()        { return Alternative_T<_TArg3, 2>(_union); }
  ConstAlternative_T<_TArg3, 2> alternative2() const  { return ConstAlternative_T<_TArg3, 2>(_union); }

  Alternative_T<_TArg4, 3>      alternative3()        { return Alternative_T<_TArg4, 3>(_union); }
  ConstAlternative_T<_TArg4, 3> alternative3() const  { return ConstAlternative_T<_TArg4, 3>(_union); }
};


/* ************************************************************************* *
 * Helper template for union of five encoders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2,
  class _TArg3, class _TArg4, class _TArg5
>
class ChoiceOfCoders5_T {
public:
  template < class _TArg, uint16_t _altIdxArg >
  class Alternative_T : public CoderAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders5_T;

    Alternative_T(util::UnionStorageAC & use_store)
      : CoderAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~Alternative_T()
    { }
  };

  template < class _TArg, uint16_t _altIdxArg >
  class ConstAlternative_T : public util::UnionConstAlternative_T<_TArg, _altIdxArg> {
  protected:
    friend class ChoiceOfCoders5_T;

    ConstAlternative_T(const util::UnionStorageAC & use_store)
      : util::UnionConstAlternative_T<_TArg, _altIdxArg>(use_store)
    { }

  public:
    ~ConstAlternative_T()
    { }
  };

  ChoiceOfCoders5_T()
  { }
  ~ChoiceOfCoders5_T()
  { }

  uint16_t getChoiceIdx(void) const { return _union.getIdx(); }

  void clear() { _union.clear(); }

  _BaseTArg * get(void)
  {
    return static_cast<_BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }
  const _BaseTArg * get(void) const
  {
    return static_cast<const _BaseTArg *>(this->_union.empty() ? 0 : this->_union.get());
  }

protected:
  util::UnionStorageOf5_T<_TArg1, _TArg2, _TArg3, _TArg4, _TArg5> _union;

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

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_CODERS_CHOICE_HPP */

