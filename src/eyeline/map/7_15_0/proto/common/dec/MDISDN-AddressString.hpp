/* ************************************************************************* *
 * ISDN-AddressString type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_DEC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_DEC_HPP

#include "eyeline/map/7_15_0/proto/common/ISDN-AddressString.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   ISDN-AddressString ::= AddressString (SIZE (1..maxISDN-AddressLength))
   maxISDN-AddressLength  INTEGER ::= 9
*/
class MDISDN_AddressString : public asn1::ber::DecoderOfOCTSTRTiny {
public:
  explicit MDISDN_AddressString(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(use_rule)
  { }
  explicit MDISDN_AddressString(ISDN_AddressString_t & use_val,
                                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfOCTSTRTiny::setValue(use_val, 9);
  }
  //
  ~MDISDN_AddressString()
  { }
};

}}}}

#endif /* __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_ENC_HPP */

