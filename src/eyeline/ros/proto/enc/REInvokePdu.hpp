/* ************************************************************************* *
 * ROS Invoke PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_INVOKE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_INVOKE_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/RELinkedIdType.hpp"
#include "eyeline/ros/proto/enc/REPduArgument.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */
class REInvokePdu : public asn1::ber::EncoderOfPlainSequence_T<4> {
private:
  using asn1::ber::EncoderOfPlainSequence_T<4>::addField;
  using asn1::ber::EncoderOfPlainSequence_T<4>::setField;
  using asn1::ber::EncoderOfPlainSequence_T<4>::clearField;

protected:
  REInvokeIdType  _invId;
  RELocalOpCode   _opCode;
  //Optional fields:
  asn1::ber::EncoderProducer_T<RELinkedIdType>  _linkedId;
  asn1::ber::EncoderProducer_T<REPduArgument>   _argument;

  //
  void construct(void);

public:
  explicit REInvokePdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<4>(use_rule)
    , _invId(use_rule), _opCode(use_rule)
  {
    construct();
  }
  REInvokePdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<4>(outer_tag, tag_env, use_rule)
    , _invId(use_rule), _opCode(use_rule)
  {
    construct();
  }
  //
  ~REInvokePdu()
  { }

  void setValue(const ROSInvokePdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_INVOKE_HPP__ */

