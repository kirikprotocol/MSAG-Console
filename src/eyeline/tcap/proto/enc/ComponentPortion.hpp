#ifndef __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_COMPONENTPORTION_HPP__

# include "eyeline/util/LWArray.hpp"

# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/tcap/proto/enc/Component.hpp"
# include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class ComponentPortion : public asn1::ber::EncoderOfSequence {
public:
  explicit ComponentPortion(asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void addComponent(const ros::ROSComponentPrimitive* ros_component);
private:
  enum { TYPICAL_MAX_NUM_OF_COMPONENTS = 4 };
  typedef util::LWArray_T<Component, uint16_t, TYPICAL_MAX_NUM_OF_COMPONENTS> ComponentsList;

  ComponentsList _sequenceOfComponents;

  enum TypeTags_e {
    ComponentPortion_Tag = 12
  };

  static const asn1::ASTagging _typeTags;
};

}}}}

#endif
