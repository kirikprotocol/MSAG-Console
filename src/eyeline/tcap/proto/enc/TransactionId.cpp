#include "TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging OrigTransactionId::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    OrigTransactionId::ORIG_TRN_ID_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging DestTransactionId::_typeTags =
    asn1::ASTagging(asn1::ASTag::tagApplication,
                    DestTransactionId::DEST_TRN_ID_Tag,
                    asn1::ASTagging::tagsIMPLICIT);

}}}}
