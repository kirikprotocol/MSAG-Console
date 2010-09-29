/* ************************************************************************* *
 * USSD-DataCodingScheme type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_DEC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_DEC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-DataCodingScheme.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   USSD-DataCodingScheme ::= OCTET STRING (SIZE (1))
*/
class MDUSSD_DataCodingScheme : public asn1::ber::DecoderOfOCTSTRTiny {
private:
  using asn1::ber::DecoderOfOCTSTRTiny::setValue;

  asn1::ber::DecoderOfOCTSTRTiny::ArrayType  _outVal;

public:
  explicit MDUSSD_DataCodingScheme(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfOCTSTRTiny(use_rule)
  { }
  MDUSSD_DataCodingScheme(USSD_DataCodingScheme_t & use_val,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfOCTSTRTiny(TSGroupBER::getTSRule(use_rule))
    , _outVal(1, &use_val, 0)
  {
    asn1::ber::DecoderOfOCTSTRTiny::setValue(_outVal, 1);
  }
  //
  ~MDUSSD_DataCodingScheme()
  { }

  void setValue(USSD_DataCodingScheme_t & use_val)
  {
    _outVal.assign(1, &use_val, 0);
    asn1::ber::DecoderOfOCTSTRTiny::setValue(_outVal, 1);
  }
};

}}}}

#endif /* __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_DEC_HPP */

