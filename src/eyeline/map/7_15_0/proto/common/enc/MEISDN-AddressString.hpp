/* ************************************************************************* *
 * ISDN-AddressString type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_ENC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_ENC_HPP

#include "eyeline/map/7_15_0/proto/common/ISDN-AddressString.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   ISDN-AddressString ::= AddressString (SIZE (1..maxISDN-AddressLength))
*/
class MEISDN_AddressString : public asn1::ber::EncoderOfOCTSTR {
protected:
  //constructor for tagged successor
  MEISDN_AddressString(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                       TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(use_tag, tag_env, TSGroupBER::getTSRule(use_rule))
  { }

public:
  explicit MEISDN_AddressString(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  { }
  MEISDN_AddressString(const ISDN_AddressString_t & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfOCTSTR::setValue(use_val);
  }
  //
  ~MEISDN_AddressString()
  { }
};

}}}}

#endif /* __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_ENC_HPP */

