/* ************************************************************************* *
 * ROS Invoke PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_INVOKE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_INVOKE_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/RELinkedIdType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= [1] SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */
class REInvokePdu : public asn1::ber::EncoderOfSequence_T<4> {
private:
  using asn1::ber::EncoderOfSequence_T<4>::addField;
  using asn1::ber::EncoderOfSequence_T<4>::setField;
  // -- OPTIONALs
  union {
    void *  _aligner;
    uint8_t _buf[sizeof(RELinkedIdType)];
  } _memLinkId;
  union {
    void *    _aligner;
    uint64_t  _buf[sizeof(asn1::ber::EncoderOfASType)];
  } _memArgType;

protected:
  REInvokeIdType  _invId;
  RELocalOpCode   _opCode;
  //Optional fields:
  RELinkedIdType *              _linkedId;
  asn1::ber::EncoderOfASType *  _argType;

  //
  void setLinkedId(const ros::LinkedId & link_id) /*throw(std::exception)*/;
  //
  void setArgType(const PDUArgument & use_arg) /*throw(std::exception)*/;

  //
  void construct(void)
  {
    asn1::ber::EncoderOfSequence_T<4>::setField(0, _invId);
    asn1::ber::EncoderOfSequence_T<4>::setField(2, _opCode);
  }
public:
  static const asn1::ASTag _pduTag; //[1] IMPLICIT

  explicit REInvokePdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<4>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _opCode(use_rule)
    , _linkedId(0), _argType(0)
  {
    _memLinkId._aligner = _memArgType._aligner = 0;
    construct();
  }
  REInvokePdu(const ROSInvokePdu & use_val,
              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequence_T<4>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _opCode(use_rule)
    , _linkedId(0), _argType(0)
  {
    _memLinkId._aligner = _memArgType._aligner = 0;
    construct();
    setValue(use_val);
  }
  //
  ~REInvokePdu()
  {
    if (_linkedId)
      _linkedId->~RELinkedIdType();
    if (_argType)
      _argType->~EncoderOfASType();
  }

  void setValue(const ROSInvokePdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_INVOKE_HPP__ */

