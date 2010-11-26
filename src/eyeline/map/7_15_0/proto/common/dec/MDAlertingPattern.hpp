/* ************************************************************************* *
 * Alerting Pattern type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_ALERTING_PATTERN_DEC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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
  explicit MDAlertingPattern(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(use_rule)
  { }
  explicit MDAlertingPattern(AlertingPattern & use_val,
                             TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
    , _outVal(1, &use_val._value, 0)
  {
    asn1::ber::DecoderOfOCTSTRTiny::setValue(_outVal, 1);
  }
  //
  ~MDAlertingPattern()
  { }

  void setValue(AlertingPattern & use_val)   /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_MAP_7F0_ALERTING_PATTERN_DEC_HPP */

