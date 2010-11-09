/* ************************************************************************* *
 * BER Encoder: encoding of ObjectID/RelativeOID subidentifiers.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SUBIDS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_SUBIDS

#include "eyeline/asn1/ObjectID.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::SubIdType;
using eyeline::asn1::RelativeOID;

using eyeline::asn1::ENCResult;
using eyeline::asn1::TSLength;
// --------------------------------------------------------------------------
// Calculates length of BER encoding of array of subIds of RELATIVE-OID or
// OBJECT-IDENTIFIER value.
// Returns number of bytes of resulted encoding or zero in case of value
// is too long and cann't be encoded.
// --------------------------------------------------------------------------
uint16_t  estimate_SubIds(const SubIdType * use_vals, RelativeOID::size_type val_num);
// --------------------------------------------------------------------------
// Encodes by BER the array of subIds of RELATIVE-OID or
// OBJECT-IDENTIFIER value according to X.690 clause 8.19.2
// --------------------------------------------------------------------------
ENCResult encode_SubIds(const SubIdType * use_vals, RelativeOID::size_type val_num,
                                uint8_t * use_enc, TSLength max_len);

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SUBIDS */

