/* ************************************************************************* *
 * ROS Invoke PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_INVOKE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_INVOKE_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDLinkedIdType.hpp"
#include "eyeline/ros/proto/dec/RDPduArgument.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */
class RDInvokePdu : public asn1::ber::DecoderOfSequence_T<4> {
private:
  using asn1::ber::DecoderOfSequence_T<4>::setField;

protected:
  ROSInvokePdu *  _dVal;

  RDInvokeIdType  _invId;
  RDLocalOpCode   _opCode;
  // -- OPTIONALs
  asn1::ber::DecoderProducer_T<RDLinkedIdType>  _linkedId;
  asn1::ber::DecoderProducer_T<RDPduArgument>   _argument;

  //Initializes ElementDecoder for this type
  void construct(void);
  //
  void setLinkedId(ros::LinkedId & link_id);
  //
  void setArgType(PDUArgument & use_arg);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { }

public:
  explicit RDInvokePdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(use_rule)
    , _dVal(0), _invId(use_rule), _opCode(use_rule)
  {
    construct();
  }
  RDInvokePdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(use_rule)
    , _dVal(0), _invId(use_rule), _opCode(use_rule)
  {
    construct();
  }
  //
  ~RDInvokePdu()
  { }

  void setValue(ROSInvokePdu & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_INVOKE_HPP__ */

