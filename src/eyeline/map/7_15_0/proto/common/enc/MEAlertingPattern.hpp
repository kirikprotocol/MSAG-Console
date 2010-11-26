/* ************************************************************************* *
 * Alerting Pattern type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_ALERTING_PATTERN_ENC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_7F0_ALERTING_PATTERN_ENC_HPP

#include "eyeline/map/7_15_0/proto/common/AlertingPattern.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   AlertingPattern ::= OCTET STRING (SIZE (1))
*/
class MEAlertingPattern : public asn1::ber::EncoderOfOCTSTR {
private:
  using asn1::ber::EncoderOfOCTSTR::setValue;

public:
  explicit MEAlertingPattern(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(use_rule)
  { }
  MEAlertingPattern(const asn1::ASTag & outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfOCTSTR(outer_tag, tag_env, use_rule)
  {}
  explicit MEAlertingPattern(const AlertingPattern & use_val,
                             TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  {
    setValue(use_val);
  }
  //
  ~MEAlertingPattern()
  { }

  void setValue(const AlertingPattern & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_MAP_7F0_ALERTING_PATTERN_ENC_HPP */

