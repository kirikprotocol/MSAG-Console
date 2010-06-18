#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDRosPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

//Initializes ElementDecoder for this type
void RDRosPdu::construct(void)
{
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(0, RDInvokePdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(1, RDRResultPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(2, RDRErrorPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(3, RDRejectPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(4, RDRResultNLPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
}
/*
void RDRosPdu::initAlt(const asn1::ASTag & problem_tag)
{
  _pDec = new (_memAlt._buf) asn1::ber::DecoderOfINTEGER(problem_tag,
                                  asn1::ASTagging::tagsIMPLICIT, getTSRule());
  _pDec->setValue(_tmpVal);
}
*/

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  RDRosPdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRosPdu : value isn't set!");
  if (unique_idx > 4) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRosPdu::prepareAlternative() : undefined UId");

  cleanUp();

  switch (unique_idx) {
  case 4:
    _pDec._resultNL = new (_memAlt._buf) RDRResultNLPdu(_dVal->initResultNL(), getVALRule());
    break;
  case 3:
    _pDec._reject = new (_memAlt._buf) RDRejectPdu(_dVal->initReject(), getVALRule());
    break;
  case 2:
    _pDec._error = new (_memAlt._buf) RDRErrorPdu(_dVal->initError(), getVALRule());
    break;
  case 1:
    _pDec._result = new (_memAlt._buf) RDRResultPdu(_dVal->initResult(), getVALRule());
    break;
  default: //case 0:
    _pDec._invoke = new (_memAlt._buf) RDInvokePdu(_dVal->initInvoke(), getVALRule());
  }
  return _pDec._any;
}


}}}}
