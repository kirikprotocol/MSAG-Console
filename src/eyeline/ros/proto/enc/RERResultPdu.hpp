/* ************************************************************************* *
 * ROS ReturnResult PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REOperationCode.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

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
class REReturnResult : public asn1::ber::EncoderOfPlainSequence_T<2> {
private:
  using asn1::ber::EncoderOfPlainSequence_T<2>::addField;
  using asn1::ber::EncoderOfPlainSequence_T<2>::setField;

protected:
  class REResultField : public asn1::ber::EncoderOfPlainSequence_T<2> {
  private:
    using asn1::ber::EncoderOfPlainSequence_T<2>::addField;
    using asn1::ber::EncoderOfPlainSequence_T<2>::setField;

  protected:
    RELocalOpCode               _opCode;
    asn1::ber::EncoderOfASType  _resType;

  public:
    REResultField(ros::LocalOpCode op_code = 0, TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::EncoderOfPlainSequence_T<2>(TSGroupBER::getTSRule(use_rule))
      , _opCode(op_code, use_rule)
      , _resType(TSGroupBER::getTSRule(use_rule))
    {
      asn1::ber::EncoderOfPlainSequence_T<2>::setField(0, _opCode);
      asn1::ber::EncoderOfPlainSequence_T<2>::setField(1, _resType);
    }
    ~REResultField()
    { }

    void setOpCode(ros::LocalOpCode op_code) { _opCode.setValue(op_code); }
    //
    void setValue(asn1::ber::TypeEncoderAC & use_result)
    {
      _resType.setValue(use_result);
    }
    void setValue(const asn1::TransferSyntax & use_ts_enc)
    {
      _resType.setValue(use_ts_enc);
    }
    void setValue(asn1::ASTypeValueAC & use_val)
    {
      TypeEncoderAC * pEnc = static_cast<TypeEncoderAC *>(use_val.getEncoder(getTSRule()));
      _resType.setValue(*pEnc);
    }
  };

  REInvokeIdType  _invId;
  REResultField * _result;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(REResultField)];
  } _memRes;


  void setResultField(ros::LocalOpCode op_code) /*throw(std::exception)*/;

  //NOTE: PDUArgument cann't be empty
  void setArgType(ros::LocalOpCode op_code, const ros::PDUArgument & use_arg)
    /*throw(std::exception)*/;

public:
  REReturnResult(const asn1::ASTag & pdu_tag,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<2>(pdu_tag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _result(0)
  {
    _memRes._aligner = 0;
    asn1::ber::EncoderOfPlainSequence_T<2>::setField(0, _invId);
  }
  ~REReturnResult()
  {
    if (_result)
      _result->~REResultField();
  }

  //
  void setValue(const ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/;
};

/* ROS ReturnResult PDU is defined in IMPLICIT tagging environment as follow:
  ReturnResult ::= [2] ReturnResultType
*/
class RERResultPdu : public REReturnResult {
protected:
  using REReturnResult::setValue;

public:
  static const asn1::ASTag _pduTag; //[2] IMPLICIT

  explicit RERResultPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : REReturnResult(_pduTag, use_rule)
  { }
  RERResultPdu(const ros::ROSResultPdu & use_val,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : REReturnResult(_pduTag, use_rule)
  {
    REReturnResult::setValue(use_val);
  }
  //
  ~RERResultPdu()
  { }

  void setValue(const ros::ROSResultPdu & use_val) /*throw(std::exception)*/
  {
    REReturnResult::setValue(use_val);
  }
};

/* ROS ReturnResultNotLast PDU is defined in IMPLICIT tagging environment as follow:
  ReturnResultNotLast ::= [7] ReturnResultType
*/
class RERResultNLPdu : public REReturnResult {
protected:
  using REReturnResult::setValue;

public:
  static const asn1::ASTag _pduTag; //[7] IMPLICIT

  explicit RERResultNLPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : REReturnResult(_pduTag, use_rule)
  { }
  RERResultNLPdu(const ros::ROSResultNLPdu & use_val,
                 TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : REReturnResult(_pduTag, use_rule)
  {
    REReturnResult::setValue(use_val);
  }
  //
  ~RERResultNLPdu()
  { }

  void setValue(const ros::ROSResultNLPdu & use_val) /*throw(std::exception)*/
  {
    REReturnResult::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_RETURN_RESULT_HPP */

