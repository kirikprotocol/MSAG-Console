/* ************************************************************************* *
 * BER Encoder methods.
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
class EncoderOfObjectID : public TLVEncoderAC {
protected:
  const ObjectID & _encVal;
  TSLength         _valSZO;

  uint16_t  calculateSubIds(void) const /*throw(std::exception)*/;

public:
  EncoderOfObjectID(const ObjectID & use_val, const ASTag * use_tag = NULL)
    : TLVEncoderAC(use_tag ? *use_tag : _tagObjectID), _encVal(use_val), _valSZO(0)
  { }
  EncoderOfObjectID(const ObjectID & use_val, const ASTagging & use_tags)
    : TLVEncoderAC(use_tags), _encVal(use_val), _valSZO(0)
  { }
  ~EncoderOfObjectID()
  { }

  // -- ************************************* --
  // -- TLVEncoderAC interface methods
  // -- ************************************* --
  //Sets required kind of BER group encoding.
  //Returns: encoding rule effective for this type.
  //NOTE: may cause recalculation of TLVLayout due to restrictions
  //      implied by the DER or CER
  TSGroupBER::Rule_e setRule(TSGroupBER::Rule_e use_rule);

  //Calculates length of BER/DER/CER encoding of type value.
  //If 'do_indef' flag is set, then length of value encoding is computed
  //even if TLVLayout uses only indefinite form of length determinants.
  //NOTE: may change TLVLayout if type value encoding should be fragmented.
  //NOTE: Throws in case of value that cann't be encoded.
  TSLength  calculateVAL(bool do_indef = false) /*throw(std::exception)*/;

  //Encodes by BER/DER/CER the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ROID */

