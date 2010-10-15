#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleSystemFailureParam.hpp"


namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTag MEExtensibleSystemFailureParam::_tag_additionalNetworkResource(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag MEExtensibleSystemFailureParam::_tag_failureCauseParam(asn1::ASTag::tagContextSpecific, 1);

void
MEExtensibleSystemFailureParam::setValue(const ExtensibleSystemFailureParam& value)
{
  //clear optionals and extensions
  clearFields();

  if (value.networkResource.get()) {
    _eNetworkResource.init(getTSRule()).setValue(*value.networkResource.get());
    setField(0, *_eNetworkResource.get());
  }

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(1, *_eExtensionContainer.get());
  }

  if (value.additionalNetworkResource.get()) {
    _eAdditionalNetworkResource.init(_tag_additionalNetworkResource, asn1::ASTagging::tagsIMPLICIT,
                                     getTSRule()).setValue(*value.additionalNetworkResource.get());
    setField(2, *_eAdditionalNetworkResource.get());
  }

  if (value.failureCauseParam.get()) {
    _eFailureCauseParam.init(_tag_failureCauseParam, asn1::ASTagging::tagsIMPLICIT,
                             getTSRule()).setValue(*value.failureCauseParam.get());
    setField(3, *_eFailureCauseParam.get());
  }

  if ( !value._unkExt._tsList.empty() )
    setExtensions(value._unkExt, 4);
}

}}}}
