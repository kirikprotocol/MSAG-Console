/* ************************************************************************* *
 * ROS ReturnResult/ReturnResultNotLast PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/ros/proto/enc/REPduArgument.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS ReturnResultType is defined in IMPLICIT tagging environment as follow:
ReturnResultType ::= SEQUENCE {
    invokeId	InvokeIdType,
    result	SEQUENCE {
      opcode	INTEGER,
      result	ABSTRACT-SYNTAX.&Type({Operations})
    } OPTIONAL
} */
class REReturnResult : public asn1::ber::EncoderOfSequence_T<2> {
protected:
  /* -- */
  class REResultField : public asn1::ber::EncoderOfSequence_T<2> {
  protected:
    RELocalOpCode   _opCode;
    REPduArgument   _resType;

  public:
    explicit REResultField(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
      : asn1::ber::EncoderOfSequence_T<2>(use_rule)
      , _opCode(use_rule), _resType(use_rule)
    {
      asn1::ber::EncoderOfSequence_T<2>::setField(0, _opCode);
      asn1::ber::EncoderOfSequence_T<2>::setField(1, _resType);
    }
    ~REResultField()
    { }
    //
    void setValue(ros::LocalOpCode op_code, const PDUArgument & op_res) /*throw(std::exception)*/
    {
      _opCode.setValue(op_code);
      _resType.setValue(op_res);
    }
  };

  REInvokeIdType  _invId;
  //Optionals:
  asn1::ber::EncoderProducer_T<REResultField>   _result;

public:
  explicit REReturnResult(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(use_rule), _invId(use_rule)
  {
    asn1::ber::EncoderOfSequence_T<2>::setField(0, _invId);
  }
  REReturnResult(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
               asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(outer_tag, tag_env, use_rule)
    , _invId(use_rule)
  {
    asn1::ber::EncoderOfSequence_T<2>::setField(0, _invId);
  }
  //
  ~REReturnResult()
  { }

  //
  void setValue(const ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP */

