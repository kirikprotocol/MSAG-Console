#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/RERosPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

void RERosPdu::cleanUp(void)
{
  if (_value._any) {
    _value._any->~TypeEncoderAC();
    _value._any = NULL;
  }
  _valTag = altNone;
}

//
REInvokePdu * RERosPdu::setInvoke(const ros::ROSInvokePdu & req_inv)
{
  cleanUp();
  _value._invoke = new (_memSelection.buf)
                    REInvokePdu(req_inv, getVALRule());
  _valTag = altInvoke;
  asn1::ber::EncoderOfChoice::setSelection(*_value._invoke);
  return _value._invoke;
}
//
RERResultPdu * RERosPdu::setResult(const ros::ROSResultPdu & req_res)
{
  cleanUp();
  _value._result = new (_memSelection.buf)
                    RERResultPdu(req_res, getVALRule());
  _valTag = altResult;
  asn1::ber::EncoderOfChoice::setSelection(*_value._resultNL);
  return _value._result;
}
//
RERResultNLPdu * RERosPdu::setResultNL(const ros::ROSResultNLPdu & req_res)
{
  cleanUp();
  _value._resultNL = new (_memSelection.buf)
                    RERResultNLPdu(req_res, getVALRule());
  _valTag = altResultNL;
  asn1::ber::EncoderOfChoice::setSelection(*_value._resultNL);
  return _value._resultNL;
}
//
RERErrorPdu * RERosPdu::setError(const ros::ROSErrorPdu & req_err)
{
  cleanUp();
  _value._error = new (_memSelection.buf)
                    RERErrorPdu(req_err, getVALRule());
  _valTag = altError;
  asn1::ber::EncoderOfChoice::setSelection(*_value._reject);
  return _value._error;
}
//
RERejectPdu * RERosPdu::setReject(const ros::ROSRejectPdu & req_rej)
{
  cleanUp();
  _value._reject = new (_memSelection.buf)
                    RERejectPdu(req_rej, getVALRule());
  _valTag = altReject;
  asn1::ber::EncoderOfChoice::setSelection(*_value._reject);
  return _value._reject;
}


//
void RERosPdu::setValue(const ros::ROSPdu & use_val) /*throw(std::exception)*/
{
  switch (use_val.getKind()) {
  case ROSPduPrimitiveAC::rosInvoke:
    setInvoke(*use_val.getInvoke()); break;
  case ROSPduPrimitiveAC::rosResult:
    setResult(*use_val.getResult()); break;
  case ROSPduPrimitiveAC::rosError:
    setError(*use_val.getError()); break;
  case ROSPduPrimitiveAC::rosReject:
    setReject(*use_val.getReject()); break;
  case ROSPduPrimitiveAC::rosResultNL:
    setResultNL(*use_val.getResultNL()); break;
  default:
    throw smsc::util::Exception("ros::proto::enc::RERosPdu::setValue() : invalid value");
  }
}

}}}}

