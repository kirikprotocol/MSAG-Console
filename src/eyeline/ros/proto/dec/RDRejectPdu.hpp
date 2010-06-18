/* ************************************************************************* *
 * ROS Reject PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_REJECT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_REJECT_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDProblemType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= [4] SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
class RDRejectPdu : public asn1::ber::DecoderOfSequence_T<2> {
private:
  using asn1::ber::DecoderOfSequence_T<2>::setField;

protected:
  ROSRejectPdu *  _dVal;
  RDInvokeIdType  _invId;
  RDProblemType   _problem;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

public:
  static const asn1::ASTag _pduTag; //[4] IMPLICIT

  explicit RDRejectPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<2>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  RDRejectPdu(ROSRejectPdu & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfSequence_T<2>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  //
  ~RDRejectPdu()
  { }

  void setValue(ROSRejectPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_REJECT_HPP__ */

