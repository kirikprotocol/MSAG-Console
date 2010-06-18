/* ************************************************************************* *
 * ROS Reject problem type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP

#include "eyeline/ros/ROSRejectProblem.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* ProblemType is defined in IMPLICIT tagging environment as follow:

ProblemType ::= CHOICE {
    general       [0]  GeneralProblem,
    invoke        [1]  InvokeProblem,
    returnResult  [2]  ReturnResultProblem,
    returnError   [3]  ReturnErrorProblem
}
*/

class REProblemType : public asn1::ber::EncoderOfChoice {
private:
  union {
    void *  _aligner;
    uint8_t _buf[sizeof(asn1::ber::EncoderOfINTEGER)];
  } _memAlt;

protected:
  static const asn1::ASTagging _tagsGeneralProblem;
  static const asn1::ASTagging _tagsInvokeProblem;
  static const asn1::ASTagging _tagsResultProblem;
  static const asn1::ASTagging _tagsErrorProblem;

  ros::RejectProblem::ProblemKind_e _altId;
  asn1::ber::EncoderOfINTEGER *     _pEnc;

  void resetAlt(void)
  {
    if (_pEnc) {
      _pEnc->~EncoderOfINTEGER();
      _pEnc = NULL;
    }
  }
  asn1::ber::EncoderOfINTEGER * initAlt(const asn1::ASTagging & problem_tags)
  {
    resetAlt();
    _pEnc = new (_memAlt._buf) asn1::ber::EncoderOfINTEGER(problem_tags, getTSRule());
    return _pEnc;
  }

public:
  explicit REProblemType(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
    , _altId(ros::RejectProblem::rejGeneral), _pEnc(0)
  {
    _memAlt._aligner = 0;
    addCanonicalAlternative(_tagsGeneralProblem);
  }
  REProblemType(const ros::RejectProblem & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
    , _altId(ros::RejectProblem::rejGeneral), _pEnc(0)
  {
    _memAlt._aligner = 0;
    addCanonicalAlternative(_tagsGeneralProblem);
    setValue(use_val);
  }
  //
  ~REProblemType()
  {
    resetAlt();
  }

  void setValue(const ros::RejectProblem & use_val) /*throw(std::exception)*/;
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_REJECT_PROBLEM_HPP */

