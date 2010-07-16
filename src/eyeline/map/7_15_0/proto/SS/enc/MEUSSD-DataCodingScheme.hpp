/* ************************************************************************* *
 * USSD-DataCodingScheme type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_ENC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_ENC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-DataCodingScheme.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
   USSD-DataCodingScheme ::= OCTET STRING (SIZE (1))
*/
class MEUSSD_DataCodingScheme : public asn1::ber::EncoderOfOCTSTR {
private:
  using asn1::ber::EncoderOfOCTSTR::setValue;

public:
  explicit MEUSSD_DataCodingScheme(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  { }
  MEUSSD_DataCodingScheme(const USSD_DataCodingScheme_t & use_val,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfOCTSTR(TSGroupBER::getTSRule(use_rule))
  {
    setValue(use_val);
  }
  //
  ~MEUSSD_DataCodingScheme()
  { }

  void setValue(const USSD_DataCodingScheme_t & use_val)
  {
    asn1::ber::EncoderOfOCTSTR::setValue((asn1::TSLength)1, &use_val);
  }
};

}}}}

#endif /* __EYELINE_MAP_7F0_USSD_DATACODINGSCHEME_ENC_HPP */

