/* ************************************************************************* *
 * ROS Reject PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_REJECT_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_REJECT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"
#include "eyeline/ros/proto/enc/REInvokeIdType.hpp"
#include "eyeline/ros/proto/enc/REProblemType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
class RERejectPdu : public asn1::ber::EncoderOfSequence_T<2> {
protected:
  REInvokeIdType  _invId;
  REProblemType   _problem;

  void construct(void);

public:
  explicit RERejectPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(use_rule)
    , _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  RERejectPdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSequence_T<2>(outer_tag, tag_env, use_rule)
    , _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  //
  ~RERejectPdu()
  { }

  void setValue(const ROSRejectPdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_REJECT_HPP__ */

