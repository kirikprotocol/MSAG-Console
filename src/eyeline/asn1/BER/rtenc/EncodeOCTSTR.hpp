/* ************************************************************************* *
 * BER Encoder methods: OCTET STRING type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ROID
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ROID

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/OCTSTR.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::OCTSTR;
using eyeline::asn1::OCTSTR_ARRAYED;
using eyeline::asn1::_tagOCTSTR;
/* ************************************************************************* *
 * Encodes by BER/DER/CER the OCTET STRING value according to X.690
 * clause 8.7 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 4] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfOCTSTR : public TypeEncoderAC {
protected:
  const OCTSTR::size_type _encValSz;
  const uint8_t *         _encVal;

public:
  EncoderOfOCTSTR(const OCTSTR & use_val, const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagOCTSTR))
    , _encValSz(use_val.size()), _encVal(use_val.get())
  { }
  EncoderOfOCTSTR(const OCTSTR_ARRAYED & use_val, const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagOCTSTR))
    , _encValSz(use_val.size()), _encVal(use_val.get())
  { }
  EncoderOfOCTSTR(const OCTSTR::size_type use_sz, const uint8_t * use_octs,
                  const ASTag * use_tag = NULL)
    : TypeEncoderAC(ASTagging(use_tag ? *use_tag : _tagOCTSTR))
    , _encValSz(use_sz), _encVal(use_octs)
  { }

  EncoderOfOCTSTR(const OCTSTR & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags)
    , _encValSz(use_val.size()), _encVal(use_val.get())
  { }
  EncoderOfOCTSTR(const OCTSTR_ARRAYED & use_val, const ASTagging & use_tags)
    : TypeEncoderAC(use_tags)
    , _encValSz(use_val.size()), _encVal(use_val.get())
  { }
  EncoderOfOCTSTR(const OCTSTR::size_type use_sz, const uint8_t * use_octs,
                  const ASTagging & use_tags)
    : TypeEncoderAC(use_tags)
    , _encValSz(use_sz), _encVal(use_octs)
  { }

  ~EncoderOfOCTSTR()
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

