/* ************************************************************************* *
 * ROS ReturnError PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= [3] SEQUENCE {
    invokeId	InvokeIdType,
    errcode	INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */
class RDRErrorPdu : public asn1::ber::DecoderOfSequence_T<3> {
private:
  using asn1::ber::DecoderOfSequence_T<3>::setField;

  union {
    void *   _aligner;
    uint8_t  _buf[sizeof(asn1::ber::DecoderOfASType)];
  } _memArg;

protected:
  ROSErrorPdu *   _dVal;
  RDInvokeIdType  _invId;
  RDLocalOpCode   _errCode;
  asn1::ber::DecoderOfASType *  _argType; //Optional

  void setArgType(PDUArgument & use_arg) /*throw(std::exception)*/;

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
  static const asn1::ASTag _pduTag; //[3] IMPLICIT

  explicit RDRErrorPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<3>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _invId(use_rule), _errCode(use_rule), _argType(0)
  {
    _memArg._aligner = 0;
    construct();
  }
  RDRErrorPdu(ROSErrorPdu & use_val,
              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<3>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _invId(use_rule), _errCode(use_rule), _argType(0)
  {
    _memArg._aligner = 0;
    construct();
  }
  //
  ~RDRErrorPdu()
  {
    if (_argType)
      _argType->~DecoderOfASType();
  }

  void setValue(ROSErrorPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP__ */

