/* ************************************************************************* *
 * ROS Reject problem type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP

#include "eyeline/ros/ROSRejectProblem.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ProblemType is defined in IMPLICIT tagging environment as follow:

ProblemType ::= CHOICE {
    general       [0]  GeneralProblem,
    invoke        [1]  InvokeProblem,
    returnResult  [2]  ReturnResultProblem,
    returnError   [3]  ReturnErrorProblem
} */
class REProblemType : public asn1::ber::EncoderOfChoice {
protected:
  using asn1::ber::EncoderOfChoice::setSelection;

  static const asn1::ASTag _tagGeneralProblem;
  static const asn1::ASTag _tagInvokeProblem;
  static const asn1::ASTag _tagResultProblem;
  static const asn1::ASTag _tagErrorProblem;

  asn1::ber::EncoderProducer_T<asn1::ber::EncoderOfINTEGER> _pEnc;

public:
  explicit REProblemType(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    addCanonicalAlternative(_tagGeneralProblem, asn1::ASTagging::tagsIMPLICIT);
  }
  REProblemType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    addCanonicalAlternative(_tagGeneralProblem, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  ~REProblemType()
  { }

  void setValue(const ros::RejectProblem & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP */

