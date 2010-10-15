/* ************************************************************************* *
 * ROS ReturnError PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/ros/proto/enc/REPduArgument.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= SEQUENCE {
    invokeId	InvokeIdType,
    errcode	    INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */
class RERErrorPdu : public asn1::ber::EncoderOfSequence_T<3> {
protected:
  REInvokeIdType  _invId;
  RELocalOpCode   _errCode;
  //Optionals
  asn1::ber::EncoderProducer_T<REPduArgument> _argType; 

  void construct(void);

public:
  explicit RERErrorPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<3>(use_rule)
    , _invId(use_rule), _errCode(use_rule)
  {
    construct();
  }
  RERErrorPdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<3>(outer_tag, tag_env, use_rule)
    , _invId(use_rule), _errCode(use_rule)
  {
    construct();
  }
  //
  ~RERErrorPdu()
  { }

  void setValue(const ROSErrorPdu & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_RETURN_ERROR_HPP__ */

