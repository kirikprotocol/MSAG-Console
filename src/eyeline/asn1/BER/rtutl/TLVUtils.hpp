/* ************************************************************************* *
 * BER Runtime: Various TLV utilities.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_UTILS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TLV_UTILS

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/rtutl/TLVProperties.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ASTag;
using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;

/* ************************************************************************* *
 * 'TL'-part decomposer 
 * ************************************************************************* */
struct TLParser : public TLVStruct {
  ASTag   _tag;

  TLParser() : TLVStruct()
  { }
  ~TLParser()
  { }

  //Decodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
  static DECResult decode_tag(ASTag & use_tag, bool & is_constructed,
                              const uint8_t * use_enc, TSLength max_len);

  //Decodes 'Length determinant'
  static DECResult decode_ld(LDeterminant & use_ld,
                             const uint8_t * use_enc, TSLength max_len);

  //Decodes 'Tag' octets of TLV encoding
  DECResult decodeTOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/;
  //Decodes 'Length' octets of TLV encoding
  DECResult decodeLOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/;
  //Decodes 'begin-of-content' octets ('T'+'L') of TLV encoding
  //NOTE: result may be DECResult::decOkRelaxed!
  DECResult decodeBOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/;
  //Decodes 'end-of-content' octets of TLV encoding
  DECResult decodeEOC(const uint8_t * use_enc, TSLength max_len) const /*throw()*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_UTILS */

