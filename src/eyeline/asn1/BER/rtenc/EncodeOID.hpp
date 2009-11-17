/* ************************************************************************* *
 * BER Encoder methods: OBJECT-IDENTIFIER type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ROID
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ROID

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/ObjectID.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ObjectID;
using eyeline::asn1::_tagObjectID;
/* ************************************************************************* *
 * Encodes by BER/DER/CER the OBJECT-IDENTIFIER value according to X.690
 * clause 8.19 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 6] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfObjectID : public TypeEncoderAC {
protected:
  const ObjectID & _encVal;
  TSLength         _valSZO;

  uint16_t  calculateSubIds(void) const /*throw(std::exception)*/;

public:
  EncoderOfObjectID(const ObjectID & use_val, const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagObjectID))
    , _encVal(use_val), _valSZO(0)
  { }
  EncoderOfObjectID(const ObjectID & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags), _encVal(use_val), _valSZO(0)
  { }
  ~EncoderOfObjectID()
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

#endif /* __ASN1_BER_ENCODER_ROID */

