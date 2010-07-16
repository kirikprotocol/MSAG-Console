/* ************************************************************************* *
 * USSD-Arg type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_ENC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_ARG_ENC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-String.hpp"
#include "eyeline/map/7_15_0/proto/common/enc/MEAlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/enc/MEISDN-AddressString.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

using eyeline::map::common::enc::MEISDN_AddressString;
using eyeline::map::common::enc::MEAlertingPattern;

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Arg ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ... ,
        alertingPattern         AlertingPattern OPTIONAL,
        msisdn              [0] ISDN-AddressString  OPTIONAL 
} */
class MEUSSD_Arg : public asn1::ber::EncoderOfSequence_T<4> {
private:
  using asn1::ber::EncoderOfSequence_T<4>::addField;
  using asn1::ber::EncoderOfSequence_T<4>::setField;
  using asn1::ber::EncoderOfSequence_T<4>::clearField;

protected:
  static const asn1::ASTag  _tag_f3;

  //Tagged Field #3 Encoder
  class FE3ISDN_AddressString : public MEISDN_AddressString {
  public:
    FE3ISDN_AddressString(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : MEISDN_AddressString(_tag_f3, asn1::ASTagging::tagsIMPLICIT, use_rule)
    { }
  };

  MEUSSD_DataCodingScheme _dcs;
  MEUSSD_String           _ussd;
  /* -- optionals -- */
  MEAlertingPattern       _alrtPtrn;
  FE3ISDN_AddressString   _msIsdn;

  //inits mandatory fields encoders
  void construct(void);

public:
  //Constructor for asn1::ASTypeValue_T<>
  MEUSSD_Arg(asn1::TransferSyntax::Rule_e use_rule)
    : asn1::ber::EncoderOfSequence_T<4>(use_rule)
    , _dcs(TSGroupBER::getBERRule(use_rule)), _ussd(TSGroupBER::getBERRule(use_rule))
    , _alrtPtrn(TSGroupBER::getBERRule(use_rule)), _msIsdn(TSGroupBER::getBERRule(use_rule))
  {
    construct();
  }
  explicit MEUSSD_Arg(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<4>(TSGroupBER::getTSRule(use_rule))
    , _dcs(use_rule), _ussd(use_rule), _alrtPtrn(use_rule), _msIsdn(use_rule)
  {
    construct();
  }
  MEUSSD_Arg(const USSD_Arg & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<4>(TSGroupBER::getTSRule(use_rule))
    , _dcs(use_rule), _ussd(use_rule), _alrtPtrn(use_rule), _msIsdn(use_rule)
  {
    construct();
    setValue(use_val);
  }
  //
  ~MEUSSD_Arg()
  { }

  void setValue(const USSD_Arg & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_MAP_7F0_USSD_ARG_ENC_HPP */

