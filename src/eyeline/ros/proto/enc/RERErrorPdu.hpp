/* ************************************************************************* *
 * ROS ReturnError PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= [3] SEQUENCE {
    invokeId	InvokeIdType,
    errcode	INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */
class RERErrorPdu : public asn1::ber::EncoderOfSequence_T<3> {
private:
  using asn1::ber::EncoderOfSequence_T<3>::addField;
  using asn1::ber::EncoderOfSequence_T<3>::setField;

  union {
    void *   _aligner;
    uint8_t  _buf[sizeof(asn1::ber::EncoderOfASType)];
  } _memArg;

protected:
  REInvokeIdType  _invId;
  RELocalOpCode   _errCode;
  asn1::ber::EncoderOfASType *  _argType; //Optional

  void setArgType(const PDUArgument & use_arg) /*throw(std::exception)*/;

  void construct(void)
  {
    asn1::ber::EncoderOfSequence_T<3>::setField(0, _invId);
    asn1::ber::EncoderOfSequence_T<3>::setField(1, _errCode);
  }

public:
  static const asn1::ASTag _pduTag; //[3] IMPLICIT

  explicit RERErrorPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<3>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _errCode(use_rule), _argType(0)
  {
    _memArg._aligner = 0;
    construct();
  }
  RERErrorPdu(const ROSErrorPdu & use_val,
              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<3>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _errCode(use_rule), _argType(0)
  {
    _memArg._aligner = 0;
    construct();
    setValue(use_val);
  }
  //
  ~RERErrorPdu()
  {
    if (_argType)
      _argType->~EncoderOfASType();
  }

  void setValue(const ROSErrorPdu & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP__ */

