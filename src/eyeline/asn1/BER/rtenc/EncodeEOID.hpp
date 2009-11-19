/* ************************************************************************* *
 * BER Encoder methods: OBJECT-IDENTIFIER type encoder, operates with encoded
 * form of OID value.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_EOID
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_EOID

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::_tagObjectID;
/* ************************************************************************* *
 * Encodes by BER/DER/CER the OBJECT-IDENTIFIER value (encoded form)
 * according to X.690 clause 8.19 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 6] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfEOID : public TypeEncoderAC {
protected:
  const EncodedOID & _encVal;

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
  EncoderOfEOID(const EncodedOID & use_val)
    : TypeEncoderAC(ASTagging(_tagObjectID))
    , _encVal(use_val)
  { }
  EncoderOfEOID(const EncodedOID & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags), _encVal(use_val)
  { }
  ~EncoderOfEOID()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_EOID */

