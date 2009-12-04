#include "Component.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging Component::_invoke_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Component::Invoke_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Component::_returnResultLast_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Component::ReturnResultLast_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Component::_returnError_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Component::ReturnError_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Component::_reject_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Component::Reject_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging Component::_returnResultNotLast_selectionTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Component::ReturnResultNotLast_SelectionTag,
                    asn1::ASTagging::tagsIMPLICIT);

Component::Component()
{
  _encoder.anyEncoder = NULL;
}

Component::Component(asn1::TSGroupBER::Rule_e use_rule)
: asn1::ber::EncoderOfChoice(use_rule)
{
  _encoder.anyEncoder = NULL;
}

void
Component::setValue(const ros::ROSInvoke* ros_component)
{
  _encoder.invokeEncoder = new (_memForEncoder.allocForInvoke) Invoke(ros_component->getInvokeId(), ros_component->opCode(), getRule());
  if ( ros_component->hasLinked() )
    _encoder.invokeEncoder->setLinkedId(ros_component->getLinked());
  _encoder.invokeEncoder->arrangeFields();
  setSelection(*_encoder.invokeEncoder, &_invoke_selectionTags);
}

void
Component::setValue(const ros::ROSResult* ros_component)
{
  _encoder.returnResultEncoder = new (_encoder.returnResultEncoder) ReturnResult(ros_component->getInvokeId(), getRule());
  _encoder.returnResultEncoder->setResult( ros_component->opCode(), NULL);
  setSelection(*_encoder.returnResultEncoder, &_returnResultLast_selectionTags);
}

void
Component::setValue(const ros::ROSError* ros_component)
{
  _encoder.returnErrorEncoder = new (_memForEncoder.allocForReturnError) ReturnError(ros_component->getInvokeId(), ros_component->opCode(), getRule());
  setSelection(*_encoder.returnErrorEncoder, &_returnError_selectionTags);
}

void
Component::setValue(const ros::ROSReject* ros_component)
{
  _encoder.rejectEncoder = new (_memForEncoder.allocForReject) Reject(ros_component->getInvokeId(), getRule());
  setSelection(*_encoder.rejectEncoder, &_reject_selectionTags);
}

void
Component::setValue(const ros::ROSResultNL* ros_component)
{
  _encoder.returnResultEncoder = new (_encoder.returnResultEncoder) ReturnResult(ros_component->getInvokeId(), getRule());
  _encoder.returnResultEncoder->setResult( ros_component->opCode(), NULL);
  setSelection(*_encoder.returnResultEncoder, &_returnResultNotLast_selectionTags);
}

}}}}
