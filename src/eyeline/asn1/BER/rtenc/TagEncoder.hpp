/* ************************************************************************* *
 * BER Encoder: Tag encoding methods.
 * ************************************************************************* */
#ifndef __ASN1_BER_TAG_ENCODER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TAG_ENCODER

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ASTag;
using eyeline::asn1::TSLength;
using eyeline::asn1::ENCResult;

/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag.
 * Returns number of bytes of resulted encoding.
 * ************************************************************************* */
extern uint8_t estimate_tag(const ASTag & use_tag);

/* ************************************************************************* *
 * Composes the 'Tag-octets' according to X.690 clause 8.1.2.
 * Returns zero in case of insuffient buffer is provided.
 * NOTE: 'use_enc' & 'max_len' arguments shouldn't be zero.
 * ************************************************************************* */
extern uint8_t compose_toc(const ASTag & use_tag, bool is_constructed,
                                   uint8_t * use_enc, uint8_t max_len);

/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
 * ************************************************************************* */
extern ENCResult encode_tag(const ASTag & use_tag, bool is_constructed,
                            uint8_t * use_enc, TSLength max_len);


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TAG_ENCODER */

