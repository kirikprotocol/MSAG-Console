/* ************************************************************************* *
 * USSD-Res type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_RES_ENC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_RES_ENC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Res.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-String.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Res ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ...
} */
class MEUSSD_Res : public asn1::ber::EncoderOfSequence_T<2> {
private:
  using asn1::ber::EncoderOfSequence_T<2>::addField;
  using asn1::ber::EncoderOfSequence_T<2>::setField;

protected:
  MEUSSD_DataCodingScheme _dcs;
  MEUSSD_String           _ussd;

  //inits mandatory fields encoders
  void construct(void);

public:
  //Constructor for asn1::ASTypeValue_T<>
  MEUSSD_Res(asn1::TransferSyntax::Rule_e use_rule)
    : asn1::ber::EncoderOfSequence_T<2>(use_rule)
    , _dcs(TSGroupBER::getBERRule(use_rule))
    , _ussd(TSGroupBER::getBERRule(use_rule))
  {
    construct();
  }
  explicit MEUSSD_Res(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(TSGroupBER::getTSRule(use_rule))
    , _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  MEUSSD_Res(const USSD_Res & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(TSGroupBER::getTSRule(use_rule))
    , _dcs(use_rule), _ussd(use_rule)
  {
    construct();
    setValue(use_val);
  }
  //
  ~MEUSSD_Res()
  { }

  void setValue(const USSD_Res & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_MAP_7F0_USSD_RES_ENC_HPP */

