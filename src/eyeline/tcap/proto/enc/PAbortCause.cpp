#include "PAbortCause.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging PAbortCause::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    PAbortCause::P_ABRT_CAUSE_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

}}}}

