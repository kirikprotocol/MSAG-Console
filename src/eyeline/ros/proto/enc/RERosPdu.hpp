/* ************************************************************************* *
 * ROS PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP

#include "eyeline/ros/ROSPdu.hpp"

#include "eyeline/ros/proto/enc/REInvokePdu.hpp"
#include "eyeline/ros/proto/enc/RERResultPdu.hpp"
#include "eyeline/ros/proto/enc/RERErrorPdu.hpp"
#include "eyeline/ros/proto/enc/RERejectPdu.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
#include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS PDU is defined in IMPLICIT TAGS environment as following CHOICE:

ROS ::= CHOICE {
  invoke              [1]  Invoke,
  returnResult        [2]  ReturnResult,
  returnError         [3]  ReturnError,
  reject              [4]  Reject
  returnResultNotLast [7]  ReturnResult
} */
class RERosPdu : public asn1::ber::EncoderOfChoice {
private:
  using asn1::ber::EncoderOfChoice::setSelection;

protected:
  class AltEncoder : public asn1::ber::ChoiceOfEncoders5_T<
                REInvokePdu, REReturnResult, RERErrorPdu, RERejectPdu, REReturnResult> {
  public:
    Alternative_T<REInvokePdu>     invoke()        { return alternative0(); }
    Alternative_T<REReturnResult>  returnResult()  { return alternative1(); }
    Alternative_T<RERErrorPdu>     returnError()   { return alternative2(); }
    Alternative_T<RERejectPdu>     reject()        { return alternative3(); }
    Alternative_T<REReturnResult>  returnResultNL()  { return alternative4(); }

    ConstAlternative_T<REInvokePdu>     invoke()        const { return alternative0(); }
    ConstAlternative_T<REReturnResult>  returnResult()  const { return alternative1(); }
    ConstAlternative_T<RERErrorPdu>     returnError()   const { return alternative2(); }
    ConstAlternative_T<RERejectPdu>     reject()        const { return alternative3(); }
    ConstAlternative_T<REReturnResult>  returnResultNL()  const { return alternative4(); }
  };

  AltEncoder  _alt;

public:
  static const asn1::ASTag  _tagInvoke;
  static const asn1::ASTag  _tagResult;
  static const asn1::ASTag  _tagError;
  static const asn1::ASTag  _tagReject;
  static const asn1::ASTag  _tagResultNL;

  explicit RERosPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(_tagInvoke, asn1::ASTagging::tagsIMPLICIT);
  }
  RERosPdu(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_tag, tag_env, use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(_tagInvoke, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  ~RERosPdu()
  { }

  //
  void setInvoke(const ros::ROSInvokePdu & use_val);
  //
  void setResult(const ros::ROSResultPdu & use_val);
  //
  void setResultNL(const ros::ROSResultNLPdu & use_val);
  //
  void setError(const ros::ROSErrorPdu & use_val);
  //
  void setReject(const ros::ROSRejectPdu & use_val);

  //
  void setValue(const ros::ROSPdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP */

