/* ************************************************************************* *
 * USSD-Arg type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-String.hpp"
#include "eyeline/map/7_15_0/proto/common/dec/MDAlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/dec/MDISDN-AddressString.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

using eyeline::map::common::dec::MDISDN_AddressString;
using eyeline::map::common::dec::MDAlertingPattern;

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Arg ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ... ,
        alertingPattern         AlertingPattern OPTIONAL,
        msisdn              [0] ISDN-AddressString  OPTIONAL
        // ... unknwon extensions
} */
class MDUSSD_Arg : public asn1::ber::DecoderOfSequence_T<5,2> {
private:
  using asn1::ber::DecoderOfSequence_T<5,2>::setField;

protected:
  USSD_Arg * _dVal;
  /* -- */
  MDUSSD_DataCodingScheme _dcs;
  MDUSSD_String           _ussd;
  /* -- optionals -- */
  asn1::ber::DecoderProducer_T<MDAlertingPattern>       _alrtPtrn;
  asn1::ber::DecoderProducer_T<MDISDN_AddressString>    _msIsdn;
  /* -- */
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension>   _uext;

  //Initializes ElementDecoder for this type
  void construct(void);
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { }

public:
  explicit MDUSSD_Arg(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5,2>(use_rule)
    , _dVal(0), _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  MDUSSD_Arg(USSD_Arg & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5,2>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _dcs(TSGroupBER::getTSRule(use_rule))
    , _ussd(TSGroupBER::getTSRule(use_rule))
  {
    construct();
  }
  //
  ~MDUSSD_Arg()
  { }

  void setValue(USSD_Arg & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};


}}}}

#endif /* __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP */

