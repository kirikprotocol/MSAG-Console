/* ************************************************************************* *
 * BER Decoder: helper template class for creation of union of TypeDecoders 
 *              in same preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODERS_CHOICE_HPP
#ident "@(#)$Id$"
#define __ASN1_BER_DECODERS_CHOICE_HPP

#include "eyeline/asn1/BER/rtutl/CodersChoiceT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Helper template for union of two decoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2
>
class ChoiceOfDecoders2_T : public ChoiceOfCoders2_T<TypeDecoderAC, _TArg1, _TArg2> {
public:
  ChoiceOfDecoders2_T() : ChoiceOfCoders2_T<TypeDecoderAC, _TArg1, _TArg2>()
  { }
  ~ChoiceOfDecoders2_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of three decoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3
>
class ChoiceOfDecoders3_T : public ChoiceOfCoders3_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3> {
public:
  ChoiceOfDecoders3_T() : ChoiceOfCoders3_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3>()
  { }
  ~ChoiceOfDecoders3_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of four decoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4
>
class ChoiceOfDecoders4_T : public ChoiceOfCoders4_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3, _TArg4> {
public:
  ChoiceOfDecoders4_T() : ChoiceOfCoders4_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3, _TArg4>()
  { }
  ~ChoiceOfDecoders4_T()
  { }
};

/* ************************************************************************* *
 * Helper template for union of five decoders ...
 * ************************************************************************* */
template <
  class _TArg1, class _TArg2, class _TArg3, class _TArg4, class _TArg5
>
class ChoiceOfDecoders5_T : public ChoiceOfCoders5_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5> {
public:
  ChoiceOfDecoders5_T() : ChoiceOfCoders5_T<TypeDecoderAC, _TArg1, _TArg2, _TArg3, _TArg4, _TArg5>()
  { }
  ~ChoiceOfDecoders5_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODERS_CHOICE_HPP */

