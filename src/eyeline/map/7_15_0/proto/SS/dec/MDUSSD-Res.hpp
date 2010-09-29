/* ************************************************************************* *
 * USSD-Res type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_USSD_RES_DEC_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_RES_DEC_HPP

#include "eyeline/map/7_15_0/proto/SS/USSD-Res.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-String.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Res ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ...
} */
class MDUSSD_Res : public asn1::ber::DecoderOfSequence_T<3> {
private:
  using asn1::ber::DecoderOfSequence_T<3>::setField;

protected:
  USSD_Res * _dVal;
  /* -- */
  MDUSSD_DataCodingScheme _dcs;
  MDUSSD_String           _ussd;
  /* -- optionals -- */
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension>   _uext;


  //Initializes ElementDecoder for this type
  void construct(void);
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

public:
  //Constructor for asn1::ASTypeValue_T<>
  explicit MDUSSD_Res(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(use_rule)
    , _dVal(0), _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  MDUSSD_Res(USSD_Res & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<3>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _dcs(TSGroupBER::getTSRule(use_rule)), _ussd(TSGroupBER::getTSRule(use_rule))
  {
    construct();
  }
  //
  ~MDUSSD_Res()
  { }

  void setValue(USSD_Res & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
  }
};


}}}}

#endif /* __EYELINE_MAP_7F0_USSD_RES_DEC_HPP */

