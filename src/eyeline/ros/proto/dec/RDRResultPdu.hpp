/* ************************************************************************* *
 * ROS ReturnResult PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/ros/proto/dec/RDPduArgument.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnResult type is defined in IMPLICIT tagging environment as follow:
ReturnResultType ::= SEQUENCE {
    invokeId	InvokeIdType,
    result	    SEQUENCE {
      opcode	INTEGER,
      result	ABSTRACT-SYNTAX.&Type({Operations})
    } OPTIONAL
} */
class RDReturnResult : public asn1::ber::DecoderOfSequence_T<2> {
protected:
  class RDResultField : public asn1::ber::DecoderOfSequence_T<2> {
  protected:
    bool            _valMask;
    RDLocalOpCode   _opCode;
    RDPduArgument   _resType;

    // ----------------------------------------
    // -- DecoderOfStructAC interface methods
    // ----------------------------------------
    //If necessary, allocates optional element and initializes associated TypeDecoderAC
    virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

  public:
    explicit RDResultField(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
      : asn1::ber::DecoderOfSequence_T<2>(use_rule)
      , _valMask(false), _opCode(use_rule), _resType(use_rule)
    {
      asn1::ber::DecoderOfSequence_T<2>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
      asn1::ber::DecoderOfSequence_T<2>::setField(1, asn1::ber::EDAlternative::altMANDATORY);
    }
    ~RDResultField()
    { }

    void setValue(ros::LocalOpCode & op_code, PDUArgument & res_arg)
    {
      _opCode.setValue(op_code);
      _resType.setValue(res_arg);
      _valMask = true;
    }
  };

  /* - */
  ros::ROSPduWithArgument * _dVal;
  RDInvokeIdType            _invId;
  //Optionals:
  asn1::ber::DecoderProducer_T<RDResultField> _result;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  explicit RDReturnResult(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(use_rule)
    , _dVal(0), _invId(use_rule)
  {
    construct();
  }
  RDReturnResult(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(outer_tag, tag_env, use_rule)
    , _dVal(0), _invId(use_rule)
  {
    construct();
  }
  //
  ~RDReturnResult()
  { }

  //
  void setValue(ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP */

