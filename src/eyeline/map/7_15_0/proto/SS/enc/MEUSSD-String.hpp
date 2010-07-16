/* ************************************************************************* *
 * USSD-String type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_STRING_ENC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_STRING_ENC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-String.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   USSD-String ::= OCTET STRING (SIZE (1..maxUSSD-StringLength))
*/
class MEUSSD_String : public asn1::ber::EncoderOfOCTSTR {
public:
  explicit MEUSSD_String(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  { }
  MEUSSD_String(const USSD_String_t & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfOCTSTR::setValue(use_val);
  }
  //
  ~MEUSSD_String()
  { }
};

}}}}

#endif /* __EYELINE_MAP_7F0_USSD_STRING_ENC_HPP */

