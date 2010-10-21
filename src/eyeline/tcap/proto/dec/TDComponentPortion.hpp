/* ************************************************************************* *
 * TCAP Message Decoder: TCAP ComponentPortion type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_COMPONENT_PORTION_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_COMPONENT_PORTION_HPP

#include "eyeline/tcap//TComponentDefs.hpp"

#include "eyeline/tcap/proto/dec/TDComponent.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeqOfLinked.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

//ComponentPortion is defined in IMPLICIT tagging environment as follow:
//
//ComponentPortion ::= [APPLICATION 12] SEQUENCE SIZE (1..MAX) OF Component

class TDComponentPortion : public asn1::ber::DecoderOfSeqOfLinked_T<ros::ROSPdu, TDComponent> {
public:
  static const asn1::ASTag _typeTag; //[APPLICATION 12] IMPLICIT

  explicit TDComponentPortion(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSeqOfLinked_T<ros::ROSPdu, TDComponent>
      (_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  ~TDComponentPortion()
  { }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_COMPONENT_PORTION_HPP */

