#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/RERosPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag  RERosPdu::_tagInvoke(asn1::ASTag::tagContextSpecific, 1);
const asn1::ASTag  RERosPdu::_tagResult(asn1::ASTag::tagContextSpecific, 2);
const asn1::ASTag  RERosPdu::_tagError(asn1::ASTag::tagContextSpecific, 3);
const asn1::ASTag  RERosPdu::_tagReject(asn1::ASTag::tagContextSpecific, 4);
const asn1::ASTag  RERosPdu::_tagResultNL(asn1::ASTag::tagContextSpecific, 7);

//
void RERosPdu::setInvoke(const ros::ROSInvokePdu & use_val)
{
  _alt.invoke().init(_tagInvoke, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  asn1::ber::EncoderOfChoice::setSelection(*_alt.get());
}
//
void RERosPdu::setResult(const ros::ROSResultPdu & use_val)
{
  _alt.returnResult().init(_tagResult, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  asn1::ber::EncoderOfChoice::setSelection(*_alt.get());
}
//
void RERosPdu::setResultNL(const ros::ROSResultNLPdu & use_val)
{
  _alt.returnResultNL().init(_tagResultNL, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  asn1::ber::EncoderOfChoice::setSelection(*_alt.get());
}
//
void RERosPdu::setError(const ros::ROSErrorPdu & use_val)
{
  _alt.returnError().init(_tagError, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  asn1::ber::EncoderOfChoice::setSelection(*_alt.get());
}
//
void RERosPdu::setReject(const ros::ROSRejectPdu & use_val)
{
  _alt.reject().init(_tagReject, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  asn1::ber::EncoderOfChoice::setSelection(*_alt.get());
}

//
void RERosPdu::setValue(const ros::ROSPdu & use_val) /*throw(std::exception)*/
{
  switch (use_val.getKind()) {
  case ROSPduPrimitiveAC::rosInvoke:
    setInvoke(*use_val.invoke().get()); break;
  case ROSPduPrimitiveAC::rosResult:
    setResult(*use_val.result().get()); break;
  case ROSPduPrimitiveAC::rosError:
    setError(*use_val.error().get()); break;
  case ROSPduPrimitiveAC::rosReject:
    setReject(*use_val.reject().get()); break;
  case ROSPduPrimitiveAC::rosResultNL:
    setResultNL(*use_val.resultNL().get()); break;
  default:
    throw smsc::util::Exception("ros::proto::enc::RERosPdu::setValue() : invalid value");
  }
}

}}}}

