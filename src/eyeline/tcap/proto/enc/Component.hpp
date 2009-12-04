#ifndef __EYELINE_TCAP_PROTO_ENC_COMPONENT_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_COMPONENT_HPP__

# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/tcap/proto/enc/Component.hpp"
# include "eyeline/tcap/proto/enc/Invoke.hpp"
# include "eyeline/tcap/proto/enc/ReturnResult.hpp"
# include "eyeline/tcap/proto/enc/ReturnError.hpp"
# include "eyeline/tcap/proto/enc/Reject.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class Component : public asn1::ber::EncoderOfChoice {
public:
  Component();
  explicit Component(asn1::TSGroupBER::Rule_e use_rule);
  void setValue(const ros::ROSInvoke* ros_component);
  void setValue(const ros::ROSResult* ros_component);
  void setValue(const ros::ROSError* ros_component);
  void setValue(const ros::ROSReject* ros_component);
  void setValue(const ros::ROSResultNL* ros_component);

private:
  union {
    void* anyEncoder;
    Invoke* invokeEncoder;
    ReturnResult* returnResultEncoder;
    ReturnError* returnErrorEncoder;
    Reject* rejectEncoder;
    ReturnResult* returnResultNotLastEncoder;
  } _encoder;

  union {
    void* aligner;
    uint8_t allocForInvoke[sizeof(Invoke)];
    uint8_t allocForReturnResult[sizeof(ReturnResult)];
    uint8_t allocForReturnError[sizeof(ReturnError)];
    uint8_t allocForReject[sizeof(Reject)];
  } _memForEncoder;

  enum SelectionTags_e {
    Invoke_SelectionTag = 1, ReturnResultLast_SelectionTag = 2, ReturnError_SelectionTag = 3,
    Reject_SelectionTag = 4, ReturnResultNotLast_SelectionTag = 7
  };
  static const asn1::ASTagging _invoke_selectionTags;
  static const asn1::ASTagging _returnResultLast_selectionTags;
  static const asn1::ASTagging _returnError_selectionTags;
  static const asn1::ASTagging _reject_selectionTags;
  static const asn1::ASTagging _returnResultNotLast_selectionTags;
};

}}}}

#endif
