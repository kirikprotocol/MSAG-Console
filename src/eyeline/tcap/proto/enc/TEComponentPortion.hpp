/* ************************************************************************* *
 * TCAP Message Encoder: TCAP ComponentPortion type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__

#include "eyeline/tcap/proto/enc/TEComponent.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeStruct.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

//ComponentPortion is defined in IMPLICIT tagging environment as follow:
//
//ComponentPortion ::= [APPLICATION 12] SEQUENCE SIZE (1..MAX) OF Component

static const uint16_t _dfltNumOfComponents = 2;
class TEComponentPortion : public asn1::ber::EncoderOfStructure_T<_dfltNumOfComponents> {
private:
  using asn1::ber::EncoderOfStructure_T<_dfltNumOfComponents>::addField;
  using asn1::ber::EncoderOfStructure_T<_dfltNumOfComponents>::setField;

protected:
  typedef util::LWArray_T<TEComponent, uint8_t, _dfltNumOfComponents> ComponentsArray;

  ComponentsArray  _comps;

public:
  static const asn1::ASTagging _typeTags;

  TEComponentPortion(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfStructure_T<_dfltNumOfComponents>(_typeTags,
                                                           TSGroupBER::getTSRule(use_rule))
  { }
  ~TEComponentPortion()
  { }

  //Creates and appends new component to list
  TEComponent * addComponent(void);
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__ */

