/* ************************************************************************* *
 * BER Encoder methods.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ENUM
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENUM

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/ENUM.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

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
class EncoderOfENUM : public TLVEncoderAC {
protected:
  const ENUM & _encVal;  //value is to encode
  TSLength     _valSZO;

public:
  EncoderOfENUM(const ENUM & use_val, const ASTag * use_tag = NULL)
    : TLVEncoderAC(use_tag ? *use_tag : _tagENUM), _encVal(use_val), _valSZO(0)
  { }
  EncoderOfENUM(const ENUM & use_val, const ASTagging & use_tags)
    : TLVEncoderAC(use_tags), _encVal(use_val), _valSZO(0)
  { }
  ~EncoderOfENUM()
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

#endif /* __ASN1_BER_ENCODER_ENUM */

