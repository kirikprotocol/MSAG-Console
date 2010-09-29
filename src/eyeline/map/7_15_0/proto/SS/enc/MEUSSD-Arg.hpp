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
#include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"

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
        // ... unknwon extensions
} */
class MEUSSD_Arg : public asn1::ber::EncoderOfPlainSequence_T<5> {
private:
  using asn1::ber::EncoderOfPlainSequence_T<5>::addField;
  using asn1::ber::EncoderOfPlainSequence_T<5>::setField;
  using asn1::ber::EncoderOfPlainSequence_T<5>::clearField;

protected:
  static const asn1::ASTag  _tag_f3;

  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;

  //Tagged Field #3 Encoder
  class FE3ISDN_AddressString : public MEISDN_AddressString {
  public:
    explicit FE3ISDN_AddressString(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
      : MEISDN_AddressString(_tag_f3, asn1::ASTagging::tagsIMPLICIT, use_rule)
    { }
    ~FE3ISDN_AddressString()
    { }
  };

  MEUSSD_DataCodingScheme _dcs;
  MEUSSD_String           _ussd;
  /* -- optionals -- */
  asn1::ber::EncoderProducer_T<MEAlertingPattern>     _eAlrtPtrn;
  asn1::ber::EncoderProducer_T<FE3ISDN_AddressString> _eMsIsdn;
  util::OptionalObj_T<MEArgUExt>                      _eUnkExt;

  //inits mandatory fields encoders
  void construct(void);

public:
  //Constructor for asn1::ASTypeValue_T<>
  explicit MEUSSD_Arg(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<5>(use_rule)
    , _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  MEUSSD_Arg(const USSD_Arg & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dcs(TSGroupBER::getTSRule(use_rule)), _ussd(TSGroupBER::getTSRule(use_rule))
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

