/* ************************************************************************* *
 * ROS PDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP

#include "eyeline/ros/proto/enc/REInvokePdu.hpp"
#include "eyeline/ros/proto/enc/RERResultPdu.hpp"
#include "eyeline/ros/proto/enc/RERErrorPdu.hpp"
#include "eyeline/ros/proto/enc/RERejectPdu.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

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

  enum Selection_e {
    altNone = 0, altInvoke = 1, altResult = 2,
    altError = 3, altReject = 4, altResultNL = 7
  };

  union {
    void *  aligner;
    uint8_t buf[eyeline::util::MaxSizeOf4_T<REInvokePdu, RERResultPdu,
                                              RERErrorPdu, RERejectPdu>::VALUE];
  } _memSelection;

protected:
  Selection_e       _valTag;
  union {
    asn1::ber::TypeEncoderAC * _any;
    REInvokePdu *   _invoke;
    RERResultPdu *  _result;
    RERErrorPdu *   _error;
    RERejectPdu *   _reject;
    RERResultNLPdu *  _resultNL;
  } _value;

  void cleanUp(void);

public:
  explicit RERosPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
    , _valTag(altNone)
  {
    _memSelection.aligner = 0;
    _value._any = 0;
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(REInvokePdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
  }
  RERosPdu(const ros::ROSPdu & use_val,
          TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
    , _valTag(altNone)
  {
    _memSelection.aligner = 0;
    _value._any = 0;
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(REInvokePdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
    setValue(use_val);
  }
  //
  ~RERosPdu()
  {
    cleanUp();
  }

  //
  REInvokePdu * setInvoke(const ros::ROSInvokePdu & req_inv);
  //
  RERResultPdu * setResult(const ros::ROSResultPdu & req_res);
  //
  RERResultNLPdu * setResultNL(const ros::ROSResultNLPdu & req_res);
  //
  RERErrorPdu * setError(const ros::ROSErrorPdu & req_err);
  //
  RERejectPdu * setReject(const ros::ROSRejectPdu & req_rej);

  //
  void setValue(const ros::ROSPdu & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_ROSPDU_HPP */

