#include "MEExtensibleSystemFailureParam.hpp"
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTag MEExtensibleSystemFailureParam::_tag_additionalNetworkResource= asn1::ASTag(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag MEExtensibleSystemFailureParam::_tag_failureCauseParam= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MEExtensibleSystemFailureParam::setValue(const ExtensibleSystemFailureParam& value)
{
  uint16_t idx=0;
  if (value.networkResource.get()) {
    _eNetworkResource.init(getTSRule()).setValue(*value.networkResource.get());
    setField(idx++, *_eNetworkResource.get());
  }
  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(idx++, *_eExtensionContainer.get());
  }
  if ( !value._unkExt._tsList.empty() )
    idx= _eUnkExt.init().setValue(value._unkExt, *this, idx);

  if (value.additionalNetworkResource.get()) {
    _eAdditionalNetworkResource.init(_tag_additionalNetworkResource, asn1::ASTagging::tagsIMPLICIT,
                                     getTSRule()).setValue(*value.additionalNetworkResource.get());
    setField(idx++, *_eAdditionalNetworkResource.get());
  }
  if (value.failureCauseParam.get()) {
    _eFailureCauseParam.init(_tag_failureCauseParam, asn1::ASTagging::tagsIMPLICIT,
                             getTSRule()).setValue(value.failureCauseParam.get()->value);
    setField(idx++, *_eFailureCauseParam.get());
  }
}

}}}}
