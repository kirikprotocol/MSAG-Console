/* ************************************************************************* *
 * ROS PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP

#include "eyeline/ros/ROSPdu.hpp"

#include "eyeline/ros/proto/dec/RDInvokePdu.hpp"
#include "eyeline/ros/proto/dec/RDRResultPdu.hpp"
#include "eyeline/ros/proto/dec/RDRErrorPdu.hpp"
#include "eyeline/ros/proto/dec/RDRejectPdu.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS PDU is defined in IMPLICIT TAGS environment as following CHOICE:

ROS ::= CHOICE {
  invoke              [1]  Invoke,
  returnResult        [2]  ReturnResult,
  returnError         [3]  ReturnError,
  reject              [4]  Reject
  returnResultNotLast [7]  ReturnResult
} */
class RDRosPdu : public asn1::ber::DecoderOfChoice_T<5> {
protected:
  class AltDecoder : public asn1::ber::ChoiceOfDecoders5_T<
                RDInvokePdu, RDReturnResult, RDRErrorPdu, RDRejectPdu, RDReturnResult> {
  public:
    Alternative_T<RDInvokePdu, 0>     invoke()        { return alternative0(); }
    Alternative_T<RDReturnResult, 1>  returnResult()  { return alternative1(); }
    Alternative_T<RDRErrorPdu, 2>     returnError()   { return alternative2(); }
    Alternative_T<RDRejectPdu, 3>     reject()        { return alternative3(); }
    Alternative_T<RDReturnResult, 4>  returnResultNL()  { return alternative4(); }

    ConstAlternative_T<RDInvokePdu, 0>     invoke()        const { return alternative0(); }
    ConstAlternative_T<RDReturnResult, 1>  returnResult()  const { return alternative1(); }
    ConstAlternative_T<RDRErrorPdu, 2>     returnError()   const { return alternative2(); }
    ConstAlternative_T<RDRejectPdu, 3>     reject()        const { return alternative3(); }
    ConstAlternative_T<RDReturnResult, 4>  returnResultNL()  const { return alternative4(); }
  };

  /* -- */
  ROSPdu *    _dVal;
  AltDecoder  _alt;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;

public:
  static const asn1::ASTag  _tagInvoke;
  static const asn1::ASTag  _tagResult;
  static const asn1::ASTag  _tagError;
  static const asn1::ASTag  _tagReject;
  static const asn1::ASTag  _tagResultNL;

  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(_tagInvoke, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagResult, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagError, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagReject, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagResultNL, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit RDRosPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(use_rule), _dVal(0)
  {
    construct();
  }
  RDRosPdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(outer_tag, tag_env, use_rule), _dVal(0)
  {
    construct();
  }
  //
  ~RDRosPdu()
  { }

  void setValue(ROSPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP */

