/* ************************************************************************* *
 * BER Encoder methods: ENUMERATED type encoder
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ENUM
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENUM

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/ENUM.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ENUM;
using eyeline::asn1::_tagENUM;
/* ************************************************************************* *
 * Encodes by BER/DER/CER the ENUMERATED value according to X.690
 * clause 8.4 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 10] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfENUM : public EncoderOfINTEGER {
public:
  EncoderOfENUM(const ENUM & use_val)
    : EncoderOfINTEGER(use_val, ASTagging(_tagENUM))
  { }
  EncoderOfENUM(const ENUM & use_val, const ASTagging & use_tags)
    : EncoderOfINTEGER(use_val, use_tags)
  { }
  ~EncoderOfENUM()
  { }
};
} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ENUM */

