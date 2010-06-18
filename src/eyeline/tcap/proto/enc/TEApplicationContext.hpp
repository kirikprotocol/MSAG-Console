/* ************************************************************************* *
 * TCAP Message Encoder: encoder of APDUs field ApplicationContextName.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_APPCTX_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_APPCTX_HPP

#include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* ApplicationContext is defined in EXPLICIT tagging environment as follow:
  ApplicationContext ::= [1] OBJECT IDENTIFIER
*/
class TEApplicationContext : public asn1::ber::EncoderOfEOID {
public:
  static const asn1::ASTagging _typeTags; //[1] EXPLICIT

  TEApplicationContext(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfEOID(_typeTags, TSGroupBER::getTSRule(use_rule))
  { }
  TEApplicationContext(const asn1::EncodedOID & use_val,
                  TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfEOID(_typeTags, TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfEOID::setValue(use_val);
  }
  ~TEApplicationContext()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_APPCTX_HPP */

