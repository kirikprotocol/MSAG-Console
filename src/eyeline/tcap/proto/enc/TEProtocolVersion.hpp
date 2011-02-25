/* ************************************************************************* *
 * TCAP Message Encoder: encoder of APDUs field ProtocolVersion.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP

#include "eyeline/tcap/proto/ProtocolVersion.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* ProtocolVersion is defined in EXPLICIT tagging environment as follow:
  ProtocolVersion ::= [0] IMPLICIT BIT STRING {version1(0)} DEFAULT {version1}
*/
class TEProtocolVersion : public asn1::ber::EncoderOfBITSTR {
private:
  using asn1::ber::EncoderOfBITSTR::setValue;

public:
  static const asn1::ASTag _typeTag; //[0] IMPLICIT

  explicit TEProtocolVersion(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfBITSTR(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    asn1::ber::EncoderOfBITSTR::setValue(proto::_dfltProtocolVersion);
  }
  //
  ~TEProtocolVersion()
  { }
  //
  void setValue(const proto::ProtocolVersion & use_val) /*throw(std::exception)*/
  {
    asn1::ber::EncoderOfBITSTR::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_PROTOVER_HPP */

