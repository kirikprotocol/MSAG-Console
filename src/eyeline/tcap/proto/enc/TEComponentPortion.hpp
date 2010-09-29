/* ************************************************************************* *
 * TCAP Message Encoder: TCAP ComponentPortion type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/enc/TEComponent.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequenced.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

//ComponentPortion is defined in IMPLICIT tagging environment as follow:
//
//ComponentPortion ::= [APPLICATION 12] SEQUENCE SIZE (1..MAX) OF Component

static const uint16_t _TMsgDfltNumOfComponents = 3;

class TEComponentPortion : public
  asn1::ber::EncoderOfSequenced_T<ros::ROSPdu, TEComponent,
                                  _TMsgDfltNumOfComponents> {
public:
  static const asn1::ASTagging _typeTags;

  explicit TEComponentPortion(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequenced_T<ros::ROSPdu, TEComponent,
                                      _TMsgDfltNumOfComponents
                                      >(_typeTags, TSGroupBER::getTSRule(use_rule))
  { }
  ~TEComponentPortion()
  { }
  //
  //EncoderOfSequenced_T<> provides method
  //void addValue(const ros::ROSPdu & use_val) /*throw(std::exception)*/;

  void addValuesList(const tcap::TComponentsPtrList & comp_list) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__ */

