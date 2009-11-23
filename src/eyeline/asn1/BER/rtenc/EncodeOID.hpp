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

  uint16_t  calculateSubIds(void) const /*throw(std::exception)*/;

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
  static const ASTagging & uniTagging(void)
  {
    static ASTagging _uniTag(_tagObjectID);
    return _uniTag;
  }

  //Constructor for untagged type referencing OBJECT IDENTIFIER
  EncoderOfObjectID(const ObjectID & use_val,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(uniTagging(), use_rule)
    , _encVal(use_val)
  { }
  //Constructor for tagged type referencing OBJECT IDENTIFIER
  EncoderOfObjectID(const ObjectID & use_val, const ASTagging & use_tags,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(use_tags, uniTagging(), use_rule), _encVal(use_val)
  { }
  ~EncoderOfObjectID()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ROID */

