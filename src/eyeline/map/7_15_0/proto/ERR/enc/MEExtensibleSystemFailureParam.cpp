#include "MEExtensibleSystemFailureParam.hpp"
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTagging MEExtensibleSystemFailureParam::_tag_additionalNetworkResource= asn1::ASTagging(asn1::ASTag::tagContextSpecific, 0,
                                                                                                      asn1::ASTagging::tagsIMPLICIT);
const asn1::ASTagging MEExtensibleSystemFailureParam::_tag_failureCauseParam= asn1::ASTagging(asn1::ASTag::tagContextSpecific, 1,
                                                                                              asn1::ASTagging::tagsIMPLICIT);

}}}}
