/* ************************************************************************* *
 * BER Encoder methods: INTEGER type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_INTEGER
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_INTEGER

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/INTEGER.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::INTEGER;
using eyeline::asn1::UINTEGER;
using eyeline::asn1::_tagINTEGER;
/* ************************************************************************* *
 * Encodes by BER/DER/CER the RELATIVE-OID value according to X.690
 * clause 8.20 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 13] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfINTEGER : public TypeEncoderAC {
protected:
  UINTEGER _encVal;  //value is to encode, negative number is 
                     //converted to 'two's complement' form
public:
  EncoderOfINTEGER(const INTEGER & use_val, const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagINTEGER))
    , _encVal(use_val)
  { }
  EncoderOfINTEGER(const INTEGER & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags), _encVal(use_val)
  { }
  EncoderOfINTEGER(const UINTEGER & use_val, const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagINTEGER))
    , _encVal(use_val)
  { }
  EncoderOfINTEGER(const UINTEGER & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags), _encVal(use_val)
  { }

  ~EncoderOfINTEGER()
  { }

  // -- ************************************* --
  // -- ValueEncoderAC interface methods
  // -- ************************************* --
  //Determines properties of addressed value encoding (LD form, constructedness)
  //according to requested encoding rule of BER family. Additionally calculates
  //length of value encoding if one of following conditions is fulfilled:
  // 1) LD form == ldDefinite
  // 2) (LD form == ldIndefinite) && ('calc_indef' == true)
  //NOTE: 'calc_indef' must be set if this encoding is enclosed by
  //another that uses definite LD form.
  //NOTE: Throws in case of value that cann't be encoded.
  const EncodingProperty & calculateVAL(bool calc_indef = false) const /*throw(std::exception)*/;
  //Encodes by requested encoding rule of BER family the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_INTEGER */

