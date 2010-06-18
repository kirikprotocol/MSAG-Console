/* ************************************************************************* *
 * ROS Invoke PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_INVOKE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_INVOKE_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDLinkedIdType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= [1] SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */
class RDInvokePdu : public asn1::ber::DecoderOfSequence_T<4> {
private:
  using asn1::ber::DecoderOfSequence_T<4>::setField;

  // -- OPTIONALs
  union {
    void *   _aligner;
    uint8_t  _buf[sizeof(RDLinkedIdType)];
  } _memLinkId;
  union {
    void *    _aligner;
    uint64_t  _buf[sizeof(asn1::ber::DecoderOfASType)];
  } _memArgType;

protected:
  ROSInvokePdu *  _dVal;

  RDInvokeIdType  _invId;
  RDLocalOpCode   _opCode;
  // -- OPTIONALs
  RDLinkedIdType *              _linkedId;
  asn1::ber::DecoderOfASType *  _argType;

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
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  static const asn1::ASTag _pduTag; //[1] IMPLICIT

  explicit RDInvokePdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<4>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _invId(use_rule), _opCode(use_rule)
    , _linkedId(0), _argType(0)
  {
    _memLinkId._aligner = _memArgType._aligner = 0;
    construct();
  }
  RDInvokePdu(ROSInvokePdu & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<4>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _invId(use_rule), _opCode(use_rule)
    , _linkedId(0), _argType(0)
  {
    _memLinkId._aligner = _memArgType._aligner = 0;
    construct();
  }
  //
  ~RDInvokePdu()
  {
    if (_linkedId)
      _linkedId->~RDLinkedIdType();
    if (_argType)
      _argType->~DecoderOfASType();
  }

  void setValue(ROSInvokePdu & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_INVOKE_HPP__ */

