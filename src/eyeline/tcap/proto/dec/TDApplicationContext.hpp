/* ************************************************************************* *
 * TCAP Message Decoder: decoder of Structured and UniDialogue APDUs field
 *                       application-context-name.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_APPCTX_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_APPCTX_HPP

#include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* ApplicationContext is defined in EXPLICIT tagging environment as follow:
  ApplicationContext ::= [1] OBJECT IDENTIFIER
*/
class TDApplicationContext : public asn1::ber::DecoderOfEOID {
public:
  static const asn1::ASTag _typeTag; //[1] EXPLICIT

  explicit TDApplicationContext(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfEOID(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                               TSGroupBER::getTSRule(use_rule))
  { }
  TDApplicationContext(asn1::EncodedOID & use_val,
                  TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfEOID(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                               TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfEOID::setValue(use_val);
  }
  ~TDApplicationContext()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_APPCTX_HPP */

