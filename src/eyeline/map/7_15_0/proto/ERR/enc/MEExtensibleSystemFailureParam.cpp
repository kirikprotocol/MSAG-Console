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
  if (value.networkResource.get()) {
    _eNetworkResource.init(getTSRule()).setValue(*value.networkResource.get());
    setField(0, *_eNetworkResource.get());
  } else
    clearField(0);

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(1, *_eExtensionContainer.get());
  } else
    clearField(1);

  if (value.additionalNetworkResource.get()) {
    _eAdditionalNetworkResource.init(_tag_additionalNetworkResource, asn1::ASTagging::tagsIMPLICIT,
                                     getTSRule()).setValue(*value.additionalNetworkResource.get());
    setField(2, *_eAdditionalNetworkResource.get());
  } else
    clearField(2);

  if (value.failureCauseParam.get()) {
    _eFailureCauseParam.init(_tag_failureCauseParam, asn1::ASTagging::tagsIMPLICIT,
                             getTSRule()).setValue(value.failureCauseParam.get()->value);
    setField(3, *_eFailureCauseParam.get());
  } else
    clearField(3);

  clearFields(4);
  if ( !value._unkExt._tsList.empty() )
    _eUnkExt.init().setValue(value._unkExt, *this, 4);
}

}}}}
