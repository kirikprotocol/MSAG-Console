/* ************************************************************************* *
 * TCAP Message Decoder: decoder of APDUs field UserInformation.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_USERINFO_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_USERINFO_HPP

#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeExternal.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeqOfLinked.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* UserInformation is defined in EXPLICIT tagging environment as follow:
  UserInformation ::= [30] IMPLICIT SEQUENCE OF EXTERNAL
*/
class TDUserInformation : public 
  asn1::ber::DecoderOfSeqOfLinked_T<asn1::ASExternal, asn1::ber::DecoderOfExternal> {
public:
  static const asn1::ASTag _typeTag; //[30] IMPLICIT

  explicit TDUserInformation(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSeqOfLinked_T<asn1::ASExternal, asn1::ber::DecoderOfExternal>
      (_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  ~TDUserInformation()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_USERINFO_HPP */

