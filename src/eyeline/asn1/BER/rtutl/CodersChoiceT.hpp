/* ************************************************************************* *
 * BER [En/de]coder: helper template class for creation of union of 
 *                   Type[En/De]coders in same preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_CODERS_CHOICE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_CODERS_CHOICE_HPP

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

#include "eyeline/util/UnionT.hpp"
#include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  class _CoderOfTArg /*: public Type[En|De]coderAC */
> 
class CoderAlternative_T : public util::UnionAlternative_T<_CoderOfTArg> {
public:
  CoderAlternative_T(util::UnionStorageAC & use_store, uint16_t use_altIdx)
    : util::UnionAlternative_T<_CoderOfTArg>(use_store, use_altIdx)
  { }
  ~CoderAlternative_T()
  { }
  //
  _CoderOfTArg  & init(TransferSyntax::Rule_e use_rule)
  {
    this->_store.clear();
    this->_store.setIdx(this->_altIdx);
    return *(new (this->_store.get())_CoderOfTArg(use_rule));
  }
  //
  _CoderOfTArg  & init(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule)
  {
    this->_store.clear();
    this->_store.setIdx(this->_altIdx);
    return *(new (this->_store.get())_CoderOfTArg(use_tag, tag_env, use_rule));
  }
};

/* ************************************************************************* *
 * Helper template for union of two [en|de]coders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2
>
class ChoiceOfCoders2_T
  : public util::ChoiceOfBased2_T<_BaseTArg, _TArg1, _TArg2,
                            CoderAlternative_T, util::UnionConstAlternative_T> {
public:
  ChoiceOfCoders2_T()
    : util::ChoiceOfBased2_T<_BaseTArg, _TArg1, _TArg2,
                        CoderAlternative_T, util::UnionConstAlternative_T>()
  { }
  ~ChoiceOfCoders2_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of three [en|de]coders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3
>
class ChoiceOfCoders3_T
  : public util::ChoiceOfBased3_T<_BaseTArg, _TArg1, _TArg2, _TArg3,
                            CoderAlternative_T, util::UnionConstAlternative_T> {
public:
  ChoiceOfCoders3_T()
    : util::ChoiceOfBased3_T<_BaseTArg, _TArg1, _TArg2, _TArg3,
                            CoderAlternative_T, util::UnionConstAlternative_T>()
  { }
  ~ChoiceOfCoders3_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of four [en|de]coders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class ChoiceOfCoders4_T
  : public util::ChoiceOfBased4_T<_BaseTArg, _TArg1, _TArg2, _TArg3, _TArg4,
                            CoderAlternative_T, util::UnionConstAlternative_T> {
public:
  ChoiceOfCoders4_T()
    : util::ChoiceOfBased4_T<_BaseTArg, _TArg1, _TArg2, _TArg3, _TArg4,
                            CoderAlternative_T, util::UnionConstAlternative_T>()
  { }
  ~ChoiceOfCoders4_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of five [en|de]coders ...
 * ************************************************************************* */
template <
  class _BaseTArg, class _TArg1, class _TArg2,
  class _TArg3, class _TArg4, class _TArg5
>
class ChoiceOfCoders5_T
  : public util::ChoiceOfBased5_T<_BaseTArg, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5,
                            CoderAlternative_T, util::UnionConstAlternative_T> {
public:
  ChoiceOfCoders5_T()
    : util::ChoiceOfBased5_T<_BaseTArg, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5,
                            CoderAlternative_T, util::UnionConstAlternative_T>()
  { }
  ~ChoiceOfCoders5_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_CODERS_CHOICE_HPP */

