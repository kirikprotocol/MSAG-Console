/* ************************************************************************* *
 * USSD-Arg type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_ENC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_7F0_USSD_ARG_ENC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-String.hpp"
#include "eyeline/map/7_15_0/proto/common/enc/MEAlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/enc/MEISDN-AddressString.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"

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
class MEUSSD_Arg : public asn1::ber::EncoderOfExtensibleSequence_T<4> {
protected:
  static const asn1::ASTag  _tag_f3;

  MEUSSD_DataCodingScheme _dcs;
  MEUSSD_String           _ussd;
  /* -- optionals -- */
  asn1::ber::EncoderProducer_T<MEAlertingPattern>     _eAlrtPtrn;
  asn1::ber::EncoderProducer_T<MEISDN_AddressString>  _eMsIsdn;

  //inits mandatory fields encoders
  void construct(void);

public:
  explicit MEUSSD_Arg(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfExtensibleSequence_T<4>(use_rule)
    , _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  explicit MEUSSD_Arg(const USSD_Arg & use_val,
            asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfExtensibleSequence_T<4>(use_rule)
    , _dcs(use_rule), _ussd(use_rule)
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

