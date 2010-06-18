/* ************************************************************************* *
 * TCAP Message Encoder: encoder of APDUs field ProtocolVersion.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP

#include "eyeline/tcap/proto/ProtocolVersion.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* ProtocolVersion is defined in EXPLICIT tagging environment as follow:
  ProtocolVersion ::= [0] IMPLICIT BIT STRING {version1(0)} DEFAULT {version1}
*/
class TEProtocolVersion : public asn1::ber::EncoderOfBITSTR {
public:
  static const asn1::ASTagging _typeTags; //[0] IMPLICIT

  explicit TEProtocolVersion(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfBITSTR(_typeTags, TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfBITSTR::setValue(proto::_dfltProtocolVersion);
  }
  TEProtocolVersion(const proto::ProtocolVersion & use_val,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfBITSTR(_typeTags, TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfBITSTR::setValue(use_val);
  }
  //
  ~TEProtocolVersion()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP */

