/* ************************************************************************* *
 * ROS PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP

#include "eyeline/ros/proto/dec/RDInvokePdu.hpp"
#include "eyeline/ros/proto/dec/RDRResultPdu.hpp"
#include "eyeline/ros/proto/dec/RDRErrorPdu.hpp"
#include "eyeline/ros/proto/dec/RDRejectPdu.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* ROS PDU is defined in IMPLICIT TAGS environment as following CHOICE:

ROS ::= CHOICE {
  invoke              [1]  Invoke,
  returnResult        [2]  ReturnResult,
  returnError         [3]  ReturnError,
  reject              [4]  Reject
  returnResultNotLast [7]  ReturnResult
} */
class RDRosPdu : public asn1::ber::DecoderOfChoice_T<5> {
private:
  using asn1::ber::DecoderOfChoice_T<5>::setAlternative;

  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf4_T<RDInvokePdu, RDRResultPdu,
                                              RDRErrorPdu, RDRejectPdu>::VALUE];
  } _memAlt;

protected:
  ROSPdu *    _dVal;
  union {
    asn1::ber::TypeDecoderAC * _any;
    RDInvokePdu *   _invoke;
    RDRResultPdu *  _result;
    RDRErrorPdu *   _error;
    RDRejectPdu *   _reject;
    RDRResultNLPdu *  _resultNL;
  } _pDec;

  void cleanUp(void)
  {
    if (_pDec._any) {
      _pDec._any->~TypeDecoderAC();
      _pDec._any = NULL;
    }
  }
  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;
  //Perfoms actions finalizing alternative decoding
  virtual void markDecodedAlternative(uint16_t unique_idx)
    /*throw(throw(std::exception)) */ { return; }

public:
  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(RDInvokePdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(RDRResultPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(RDRErrorPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(RDRejectPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(RDRResultNLPdu::_pduTag, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit RDRosPdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfChoice_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(0)
  {
    _memAlt._aligner = 0;
    _pDec._any = 0;
    construct();
  }
  RDRosPdu(ROSPdu & use_val,
           TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfChoice_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val)
  {
    _memAlt._aligner = 0;
    _pDec._any = 0;
    construct();
  }
  //
  ~RDRosPdu()
  {
    cleanUp();
  }

  void setValue(ROSPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_ROSPDU_HPP */

