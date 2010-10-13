/* ************************************************************************* *
 * BER Encoder: helper template class for creation of union of TypeEncoders 
 *              in same preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODERS_CHOICE_HPP
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODERS_CHOICE_HPP

#include "eyeline/asn1/BER/rtutl/CodersChoiceT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Helper template for union of two encoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2
>
class ChoiceOfEncoders2_T : public ChoiceOfCoders2_T<TypeEncoderAC, _TArg1, _TArg2> {
public:
  ChoiceOfEncoders2_T() : ChoiceOfCoders2_T<TypeEncoderAC, _TArg1, _TArg2>()
  { }
  ~ChoiceOfEncoders2_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of three encoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3
>
class ChoiceOfEncoders3_T : public ChoiceOfCoders3_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3> {
public:
  ChoiceOfEncoders3_T() : ChoiceOfCoders3_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3>()
  { }
  ~ChoiceOfEncoders3_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of four encoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class ChoiceOfEncoders4_T : public ChoiceOfCoders4_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3, _TArg4> {
public:
  ChoiceOfEncoders4_T() : ChoiceOfCoders4_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3, _TArg4>()
  { }
  ~ChoiceOfEncoders4_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of five encoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
>
class ChoiceOfEncoders5_T : public ChoiceOfCoders5_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5> {
public:
  ChoiceOfEncoders5_T() : ChoiceOfCoders5_T<TypeEncoderAC, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5>()
  { }
  ~ChoiceOfEncoders5_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODERS_CHOICE_HPP */

