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
#include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"

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
class MEUSSD_Res : public asn1::ber::EncoderOfPlainSequence_T<3> {
private:
  using asn1::ber::EncoderOfPlainSequence_T<3>::addField;
  using asn1::ber::EncoderOfPlainSequence_T<3>::setField;

protected:
  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;

  MEUSSD_DataCodingScheme _dcs;
  MEUSSD_String           _ussd;
  /* -- optionals -- */
  util::OptionalObj_T<MEArgUExt>  _eUnkExt;

  //inits mandatory fields encoders
  void construct(void);

public:
  explicit MEUSSD_Res(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<3>(use_rule)
    , _dcs(use_rule), _ussd(use_rule)
  {
    construct();
  }
  MEUSSD_Res(const USSD_Res & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<3>(TSGroupBER::getTSRule(use_rule))
    , _dcs(TSGroupBER::getTSRule(use_rule)), _ussd(TSGroupBER::getTSRule(use_rule))
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

