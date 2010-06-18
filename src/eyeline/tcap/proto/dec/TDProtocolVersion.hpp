/* ************************************************************************* *
 * TCAP Message Decoder: decoder of APDUs field ProtocolVersion.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP

#include "eyeline/tcap/proto/ProtocolVersion.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeBITSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* ProtocolVersion is defined in EXPLICIT tagging environment as follow:
  ProtocolVersion ::= [0] IMPLICIT BIT STRING {version1(0)} DEFAULT {version1}
*/
class TDProtocolVersion : public asn1::ber::DecoderOfBITSTR_T<uint8_t> {
private:
  using asn1::ber::DecoderOfBITSTR_T<uint8_t>::setValue;

public:
  static const asn1::ASTag _typeTag; //[0] IMPLICIT

  explicit TDProtocolVersion(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfBITSTR_T<uint8_t>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                            TSGroupBER::getTSRule(use_rule))
  { }
  TDProtocolVersion(proto::ProtocolVersion & use_val,
                  TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfBITSTR_T<uint8_t>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                            TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfBITSTR_T<uint8_t>::setValue(use_val);
  }
  ~TDProtocolVersion()
  { }

  void setValue(proto::ProtocolVersion & use_val)
  {
    asn1::ber::DecoderOfBITSTR_T<uint8_t>::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP */

