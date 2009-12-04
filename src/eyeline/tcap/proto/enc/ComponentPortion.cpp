#include "ComponentPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging ComponentPortion::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    ComponentPortion::ComponentPortion_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

ComponentPortion::ComponentPortion(asn1::TSGroupBER::Rule_e use_rule)
: EncoderOfSequence(_typeTags, use_rule)
{}

void
ComponentPortion::addComponent(const ros::ROSComponentPrimitive* ros_component)
{
  Component rosComponent(getRule());
  switch(ros_component->rosKind()) {
  case ros::ROSComponentPrimitive::rosInvoke:
    rosComponent.setValue(static_cast<const ros::ROSInvoke*>(ros_component));
    break;
  case ros::ROSComponentPrimitive::rosResult:
    rosComponent.setValue(static_cast<const ros::ROSResult*>(ros_component));
    break;
  case ros::ROSComponentPrimitive::rosError:
    rosComponent.setValue(static_cast<const ros::ROSError*>(ros_component));
    break;
  case ros::ROSComponentPrimitive::rosReject:
    rosComponent.setValue(static_cast<const ros::ROSReject*>(ros_component));
    break;
  case ros::ROSComponentPrimitive::rosResultNL:
    rosComponent.setValue(static_cast<const ros::ROSResultNL*>(ros_component));
    break;
  default:
    throw utilx::SerializationException("ComponentPortion::addComponent::: invalid ros component type=%u",
                                        ros_component->rosKind());
  }

  _sequenceOfComponents.append(rosComponent);
  addField(_sequenceOfComponents[_sequenceOfComponents.size() - 1]);
}


}}}}
