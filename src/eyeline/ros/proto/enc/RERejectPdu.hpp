/* ************************************************************************* *
 * ROS Reject PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_REJECT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_REJECT_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REProblemType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= [4] SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
class RERejectPdu : public asn1::ber::EncoderOfPlainSequence_T<2> {
private:
  using asn1::ber::EncoderOfPlainSequence_T<2>::addField;
  using asn1::ber::EncoderOfPlainSequence_T<2>::setField;

protected:
  REInvokeIdType  _invId;
  REProblemType   _problem;

  void construct(void)
  {
    asn1::ber::EncoderOfPlainSequence_T<2>::setField(0, _invId);
    asn1::ber::EncoderOfPlainSequence_T<2>::setField(1, _problem);
  }

public:
  static const asn1::ASTag _pduTag; //[4] IMPLICIT

  explicit RERejectPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<2>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  RERejectPdu(const ROSRejectPdu & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainSequence_T<2>(_pduTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _invId(use_rule), _problem(use_rule)
  {
    construct();
    setValue(use_val);
  }
  //
  ~RERejectPdu()
  { }

  void setValue(const ROSRejectPdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_REJECT_HPP__ */

