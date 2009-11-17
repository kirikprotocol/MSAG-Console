/* ************************************************************************* *
 * BER Decoder: 'TL' parts decoding methods.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_DECODER
#ident "@(#)$Id$"
#define __ASN1_BER_TLV_DECODER

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTagging;
using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;

/* ************************************************************************* *
 * Decodes by BER the ASN.1 Tag according to X.680 clause 8.1.2. 
 * Returns number of bytes processed from encoding or zero in case of failure.
 * ************************************************************************* */
DECResult decode_tag(ASTag & use_tag, bool & is_constructed, 
                     const uint8_t * use_enc, TSLength enc_len);

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_DECODER */

