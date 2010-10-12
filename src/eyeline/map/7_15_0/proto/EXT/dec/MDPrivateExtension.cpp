#include "MDPrivateExtension.hpp"
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

void
MDPrivateExtension::construct(void)
{
  setField(0, asn1::_tagObjectID, asn1::ber::EDAlternative::altMANDATORY);
  setField(1, asn1::_tagANYTYPE, asn1::ber::EDAlternative::altOPTIONAL);
}

/*
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/
asn1::ber::TypeDecoderAC*
MDPrivateExtension::prepareAlternative(uint16_t unique_idx)
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDPrivateExtension::prepareAlternative: value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("MDPrivateExtension::prepareAlternative: undefined UId");

  if (unique_idx == 0) {
    _dExtId.setValue(_value->extId);
    return &_dExtId;
  } else {
    _dExtType.init(getTSRule()).setValue(_value->extType.init());
    return _dExtType.get();
  }
}

}}}}
