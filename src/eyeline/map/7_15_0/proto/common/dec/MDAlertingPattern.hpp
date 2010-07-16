/* ************************************************************************* *
 * Alerting Pattern type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_ALERTING_PATTERN_DEC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_ALERTING_PATTERN_DEC_HPP

#include "eyeline/map/7_15_0/proto/common/AlertingPattern.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   AlertingPattern ::= OCTET STRING (SIZE (1))
*/
class MDAlertingPattern : public asn1::ber::DecoderOfOCTSTRTiny {
private:
  using asn1::ber::DecoderOfOCTSTRTiny::setValue;

  asn1::ber::DecoderOfOCTSTRTiny::ArrayType  _outVal;

public:
  explicit MDAlertingPattern(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
  { }
  MDAlertingPattern(AlertingPattern::value_type & use_val,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
    , _outVal(1, &use_val, 0)
  {
    asn1::ber::DecoderOfOCTSTRTiny::setValue(_outVal, 1);
  }
  //
  ~MDAlertingPattern()
  { }

  void setValue(AlertingPattern::value_type & use_val)   /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_MAP_7F0_ALERTING_PATTERN_DEC_HPP */

