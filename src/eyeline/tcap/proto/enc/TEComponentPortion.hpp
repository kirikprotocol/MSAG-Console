/* ************************************************************************* *
 * TCAP Message Encoder: TCAP ComponentPortion type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/enc/TEComponent.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSeqOfLinked.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

//ComponentPortion is defined in IMPLICIT tagging environment as follow:
//
//ComponentPortion ::= [APPLICATION 12] SEQUENCE SIZE (1..MAX) OF Component

static const uint16_t _TMsgDfltNumOfComponents = 3;

class TEComponentPortion : public
  asn1::ber::EncoderOfSeqOfLinked_T<ros::ROSPdu, TEComponent, _TMsgDfltNumOfComponents> {
public:
  static const asn1::ASTagging _typeTags;

  explicit TEComponentPortion(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSeqOfLinked_T<ros::ROSPdu, TEComponent,
                                      _TMsgDfltNumOfComponents>(_typeTags, use_rule)
  { }
  ~TEComponentPortion()
  { }

  //void setValue(const tcap::TComponentsPtrList & comp_list) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__ */

