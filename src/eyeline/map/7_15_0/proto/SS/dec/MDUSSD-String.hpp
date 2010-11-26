/* ************************************************************************* *
 * USSD-String type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_STRING_DEC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_7F0_USSD_STRING_DEC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-String.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   USSD-String ::= OCTET STRING (SIZE (1..maxUSSD-StringLength))
*/
class MDUSSD_String : public asn1::ber::DecoderOfOCTSTRTiny {
public:
  explicit MDUSSD_String(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(use_rule)
  { }
  MDUSSD_String(USSD_String_t & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfOCTSTRTiny::setValue(use_val);
  }
  //
  ~MDUSSD_String()
  { }
};

}}}}

#endif /* __EYELINE_MAP_7F0_USSD_STRING_DEC_HPP */

