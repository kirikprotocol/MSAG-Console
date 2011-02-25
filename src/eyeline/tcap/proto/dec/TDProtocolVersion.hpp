/* ************************************************************************* *
 * TCAP Message Decoder: decoder of APDUs field ProtocolVersion.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP

#include "eyeline/tcap/proto/ProtocolVersion.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeBITSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ProtocolVersion is defined in EXPLICIT tagging environment as follow:
  ProtocolVersion ::= [0] IMPLICIT BIT STRING {version1(0)} DEFAULT {version1}
*/
class TDProtocolVersion : public asn1::ber::DecoderOfBITSTR_T<uint8_t> {
public:
  static const asn1::ASTag _typeTag; //[0] IMPLICIT

  explicit TDProtocolVersion(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfBITSTR_T<uint8_t>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  explicit TDProtocolVersion(proto::ProtocolVersion & use_val,
                             asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfBITSTR_T<uint8_t>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    asn1::ber::DecoderOfBITSTR_T<uint8_t>::setValue(use_val);
  }
  ~TDProtocolVersion()
  { }

  //Base class provides
  //void setValue(proto::ProtocolVersion & use_val);
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_PROTOVER_HPP */

