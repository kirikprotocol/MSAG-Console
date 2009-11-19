/* ************************************************************************* *
 * BER Encoder methods: BIT STRING type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_BITSTR
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_BITSTR

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BITSTR.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::BITSTR;
using eyeline::asn1::_tagBITSTR;
using eyeline::util::bitsNum2Octs;

/* ************************************************************************* *
 * Encodes by BER/DER/CER the BIT STRING value according to X.690
 * clause 8.7 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 3] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfBITSTR : public TypeEncoderAC {
protected:
  const BITSTR::size_type _encValSz;
  const BITSTR::size_type _encValBits;
  const uint8_t *         _encVal;


  //Returns number of unused bits in last octet
  uint8_t unusedBits(void) const { return (uint8_t)(8U - _encValBits%8); }

  BITSTR::size_type numFragments(void) const
  {
    //NOTE: one byte of 1000 bytes fragment is taken by unused bits marker
    return (_encValSz/999 + (_encValSz%999 ? 1 : 0));
  }

  //Returns total length of fragmented encoding
  //Throws if value is too large and cann't be encoded
  TSLength calculateFragments(void) const /*throw(std::exception)*/;

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
  const EncodingProperty & calculateVAL(bool calc_indef = false) /*throw(std::exception)*/;
  //Encodes by requested encoding rule of BER family the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  //NOTE: this method has defined result only after calculateVAL() called
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/;

public:
  EncoderOfBITSTR(const BITSTR & use_val)
    : TypeEncoderAC(ASTagging(_tagBITSTR))
    , _encValBits(use_val.size()), _encVal(use_val.getOcts())
    , _encValSz(bitsNum2Octs(_encValBits))
  { }
  EncoderOfBITSTR(const BITSTR::size_type num_bits, const uint8_t * use_octs)
    : TypeEncoderAC(ASTagging(_tagBITSTR))
    , _encValBits(num_bits), _encVal(use_octs)
    , _encValSz(bitsNum2Octs(_encValBits))
  { }

  EncoderOfBITSTR(const BITSTR & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags)
    , _encValBits(use_val.size()), _encVal(use_val.getOcts())
    , _encValSz(bitsNum2Octs(_encValBits))
  { }
  EncoderOfBITSTR(const BITSTR::size_type num_bits, const uint8_t * use_octs,
                  const ASTagging & use_tags)
    : TypeEncoderAC(use_tags)
    , _encValBits(num_bits), _encVal(use_octs)
    , _encValSz(bitsNum2Octs(_encValBits))
  { }

  ~EncoderOfBITSTR()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_BITSTR */

