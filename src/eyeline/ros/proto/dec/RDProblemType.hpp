/* ************************************************************************* *
 * ROS Reject problem type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_REJECT_PROBLEM_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_REJECT_PROBLEM_HPP

#include "eyeline/ros/ROSRejectProblem.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ProblemType is defined in IMPLICIT tagging environment as follow:

ProblemType ::= CHOICE {
    general       [0]  GeneralProblem,
    invoke        [1]  InvokeProblem,
    returnResult  [2]  ReturnResultProblem,
    returnError   [3]  ReturnErrorProblem
} */
class RDProblemType : public asn1::ber::DecoderOfChoice_T<4> {
private:
  uint8_t  _tmpVal;

protected:
  ros::RejectProblem *  _dVal;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfINTEGER> _pDec;

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;
  //Perfoms actions finalizing alternative decoding
  virtual void markDecodedAlternative(uint16_t unique_idx)
    /*throw(throw(std::exception)) */;

  //Initializes ElementDecoder for this type
  void construct(void);
  //
  void initAlt(const asn1::ASTag & problem_tag);

public:
  static const asn1::ASTag _tagGeneralProblem;
  static const asn1::ASTag _tagInvokeProblem;
  static const asn1::ASTag _tagResultProblem;
  static const asn1::ASTag _tagErrorProblem;

  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(_tagGeneralProblem, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagInvokeProblem, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagResultProblem, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagErrorProblem, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit RDProblemType(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<4>(use_rule)
    , _tmpVal(0), _dVal(0)
  {
    construct();
  }
  RDProblemType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<4>(outer_tag, tag_env, use_rule)
    , _tmpVal(0), _dVal(0)
  {
    construct();
  }
  //
  ~RDProblemType()
  { }

  void setValue(ros::RejectProblem & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_REJECT_PROBLEM_HPP */

