/* ************************************************************************* *
 * ROS ReturnResult PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnResult type is defined in IMPLICIT tagging environment as follow:
ReturnResultType ::= SEQUENCE {
    invokeId	InvokeIdType,
    result	SEQUENCE {
      opcode	INTEGER,
      result	ABSTRACT-SYNTAX.&Type({Operations})
    } OPTIONAL
} */
class RDReturnResult : public asn1::ber::DecoderOfSequence_T<2> {
private:
  using asn1::ber::DecoderOfSequence_T<2>::setField;

  class RDResultField : public asn1::ber::DecoderOfSequence_T<2> {
  private:
    using asn1::ber::DecoderOfSequence_T<2>::setField;

    bool  _valMask;

  protected:
    RDLocalOpCode               _opCode;
    asn1::ber::DecoderOfASType  _resType;

    // ----------------------------------------
    // -- DecoderOfStructAC interface methods
    // ----------------------------------------
    //If necessary, allocates optional element and initializes associated TypeDecoderAC
    virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
    //Performs actions upon successfull optional element decoding
    virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

  public:
    explicit RDResultField(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::DecoderOfSequence_T<2>(TSGroupBER::getTSRule(use_rule))
      , _valMask(false), _opCode(use_rule), _resType(TSGroupBER::getTSRule(use_rule))
    {
      asn1::ber::DecoderOfSequence_T<2>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
      asn1::ber::DecoderOfSequence_T<2>::setField(1, asn1::ber::EDAlternative::altMANDATORY);
    }
    ~RDResultField()
    { }

    void setValue(ros::LocalOpCode & op_code, asn1::TransferSyntax & use_ts_enc)
    {
      _opCode.setValue(op_code);
      _resType.setValue(use_ts_enc);
      _valMask = true;
    }
  };

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(RDResultField)];
  } _memRes;

protected:
  ros::ROSPduWithArgument * _dVal;
  RDInvokeIdType            _invId;
  RDResultField *           _result;

  //NOTE: PDUArgument cann't be empty
  void setResultField(ros::LocalOpCode & op_code, 
                      ros::PDUArgument & use_arg) /*throw(std::exception)*/;

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  RDReturnResult(const asn1::ASTag & pdu_tag,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<2>(pdu_tag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _invId(use_rule), _result(0)
  {
    _memRes._aligner = 0;
    asn1::ber::DecoderOfSequence_T<2>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
    asn1::ber::DecoderOfSequence_T<2>::setField(1, asn1::_tagSEQOF, asn1::ber::EDAlternative::altOPTIONAL);
  }
  ~RDReturnResult()
  {
    if (_result)
      _result->~RDResultField();
  }

  //
  void setValue(ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

/* ROS ReturnResult PDU is defined in IMPLICIT tagging environment as follow:
  ReturnResult ::= [2] ReturnResultType
*/
class RDRResultPdu : public RDReturnResult {
protected:
  using RDReturnResult::setValue;

public:
  static const asn1::ASTag _pduTag; //[2] IMPLICIT

  explicit RDRResultPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : RDReturnResult(_pduTag, use_rule)
  { }
  RDRResultPdu(ros::ROSResultPdu & use_val,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : RDReturnResult(_pduTag, use_rule)
  {
    RDReturnResult::setValue(use_val);
  }
  //
  ~RDRResultPdu()
  { }

  void setValue(ros::ROSResultPdu & use_val) /*throw(std::exception)*/
  {
    RDReturnResult::setValue(use_val);
  }
};

/* ROS ReturnResultNotLast PDU is defined in IMPLICIT tagging environment as follow:
  ReturnResultNotLast ::= [7] ReturnResultType
*/
class RDRResultNLPdu : public RDReturnResult {
protected:
  using RDReturnResult::setValue;

public:
  static const asn1::ASTag _pduTag; //[7] IMPLICIT

  explicit RDRResultNLPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : RDReturnResult(_pduTag, use_rule)
  { }
  RDRResultNLPdu(ros::ROSResultNLPdu & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : RDReturnResult(_pduTag, use_rule)
  {
    RDReturnResult::setValue(use_val);
  }
  //
  ~RDRResultNLPdu()
  { }

  void setValue(ros::ROSResultNLPdu & use_val) /*throw(std::exception)*/
  {
    RDReturnResult::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_RETURN_RESULT_HPP */

