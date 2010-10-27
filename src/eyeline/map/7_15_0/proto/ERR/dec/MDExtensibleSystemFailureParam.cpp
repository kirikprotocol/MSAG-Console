#include "MDExtensibleSystemFailureParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

const asn1::ASTag
MDExtensibleSystemFailureParam::_tag_additionalNetworkResource= asn1::ASTag(asn1::ASTag::tagContextSpecific, 0);

const asn1::ASTag
MDExtensibleSystemFailureParam::_tag_failureCauseParam= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MDExtensibleSystemFailureParam::construct()
{
  setField(0, asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, asn1::_tagSEQOF, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(2, _tag_additionalNetworkResource, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(3, _tag_failureCauseParam, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setUnkExtension(4);
}

/*
ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource NetworkResource OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource       [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam       [1] FailureCauseParam   OPTIONAL }
*/
asn1::ber::TypeDecoderAC*
MDExtensibleSystemFailureParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDExtensibleSystemFailureParam::prepareAlternative: value isn't set!");
  if (unique_idx > 4)
    throw smsc::util::Exception("MDExtensibleSystemFailureParam::prepareAlternative: undefined UId");

  if (unique_idx == 0) {
    _dNetworkResource.init(getTSRule()).setValue(_value->networkResource.init());
    return _dNetworkResource.get();
  }
  if (unique_idx == 1) {
    _dExtContainer.init(getTSRule()).setValue(_value->extensionContainer.init());
    return _dExtContainer.get();
  }
  if (unique_idx == 2) {
    _dAdditionalNetworkResource.init(getTSRule()).setValue(_value->additionalNetworkResource.init());
    return _dAdditionalNetworkResource.get();
  }
  if (unique_idx == 3) {
    _dFailureCauseParam.init(getTSRule()).setValue(_value->failureCauseParam.init());
    return _dFailureCauseParam.get();
  }
  _uext.init(getTSRule()).setValue(_value->_unkExt);
  return _uext.get();
}

}}}}
