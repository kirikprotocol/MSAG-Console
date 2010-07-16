/* ************************************************************************* *
 * USSD-Arg type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-String.hpp"
#include "eyeline/map/7_15_0/proto/common/dec/MDAlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/dec/MDISDN-AddressString.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"

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
} */
class MDUSSD_Arg : public asn1::ber::DecoderOfSequence_T<5> {
private:
  using asn1::ber::DecoderOfSequence_T<5>::setField;

protected:
  USSD_Arg * _dVal;
  /* -- */
  MDUSSD_DataCodingScheme _dcs;
  MDUSSD_String           _ussd;
  /* -- optionals -- */
  MDAlertingPattern       _alrtPtrn;
  MDISDN_AddressString    _msIsdn;
  /* -- */
  asn1::ber::DecoderOfUExtension   _uext;

  //Initializes ElementDecoder for this type
  void construct(void);
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  //Constructor for asn1::ASTypeValue_T<>
  MDUSSD_Arg(asn1::TransferSyntax::Rule_e use_rule)
    : asn1::ber::DecoderOfSequence_T<5>(use_rule)
    , _dVal(0), _dcs(TSGroupBER::getBERRule(use_rule))
    , _ussd(TSGroupBER::getBERRule(use_rule))
    , _alrtPtrn(TSGroupBER::getBERRule(use_rule))
    , _msIsdn(TSGroupBER::getBERRule(use_rule)), _uext(use_rule)
  {
    construct();
  }
  explicit MDUSSD_Arg(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _dcs(use_rule), _ussd(use_rule), _alrtPtrn(use_rule)
    , _msIsdn(use_rule), _uext(TSGroupBER::getTSRule(use_rule))
  {
    construct();
  }
  MDUSSD_Arg(USSD_Arg & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _dcs(use_rule), _ussd(use_rule), _alrtPtrn(use_rule)
    , _msIsdn(use_rule), _uext(TSGroupBER::getTSRule(use_rule))
  {
    construct();
  }
  //
  ~MDUSSD_Arg()
  { }

  void setValue(USSD_Arg & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
  }
};


}}}}

#endif /* __EYELINE_MAP_7F0_USSD_ARG_DEC_HPP */

