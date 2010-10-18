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

/* ApplicationContext is defined in EXPLICIT tagging environment as follow:
  ApplicationContext ::= [1] OBJECT IDENTIFIER
*/
class TEApplicationContext : public asn1::ber::EncoderOfEOID {
public:
  static const asn1::ASTag _typeTag; //[1] EXPLICIT

  explicit TEApplicationContext(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfEOID(_typeTag, asn1::ASTagging::tagsEXPLICIT, use_rule)
  { }
  explicit TEApplicationContext(const asn1::EncodedOID & use_val,
                  asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfEOID(_typeTag, asn1::ASTagging::tagsEXPLICIT, use_rule)
  {
    asn1::ber::EncoderOfEOID::setValue(use_val);
  }
  ~TEApplicationContext()
  { }

  //void setValue(const asn1::EncodedOID & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_APPCTX_HPP */

