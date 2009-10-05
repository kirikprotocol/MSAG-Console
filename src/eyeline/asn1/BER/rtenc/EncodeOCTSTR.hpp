/* ************************************************************************* *
 * BER Encoder methods.
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
class EncoderOfOCTSTR : public TLVEncoderAC {
protected:
  const OCTSTR::size_type _encValSz;
  const uint8_t *         _encVal;
  TSLength                _valSZO;

public:
  EncoderOfOCTSTR(const OCTSTR & use_val, const ASTag * use_tag = NULL)
    : TLVEncoderAC(use_tag ? *use_tag : _tagOCTSTR)
    , _encValSz(use_val.size()), _encVal(use_val.get()), _valSZO(0)
  { }
  EncoderOfOCTSTR(const OCTSTR_ARRAYED & use_val, const ASTag * use_tag = NULL)
    : TLVEncoderAC(use_tag ? *use_tag : _tagOCTSTR)
    , _encValSz(use_val.size()), _encVal(use_val.get()), _valSZO(0)
  { }
  EncoderOfOCTSTR(const OCTSTR::size_type use_sz, const uint8_t * use_octs,
                  const ASTag * use_tag = NULL)
    : TLVEncoderAC(use_tag ? *use_tag : _tagOCTSTR)
    , _encValSz(use_sz), _encVal(use_octs), _valSZO(0)
  { }

  EncoderOfOCTSTR(const OCTSTR & use_val, const ASTagging & use_tags)
    : TLVEncoderAC(use_tags)
    , _encValSz(use_val.size()), _encVal(use_val.get()), _valSZO(0)
  { }
  EncoderOfOCTSTR(const OCTSTR_ARRAYED & use_val, const ASTagging & use_tags)
    : TLVEncoderAC(use_tags)
    , _encValSz(use_val.size()), _encVal(use_val.get()), _valSZO(0)
  { }
  EncoderOfOCTSTR(const OCTSTR::size_type use_sz, const uint8_t * use_octs,
                  const ASTagging & use_tags)
    : TLVEncoderAC(use_tags)
    , _encValSz(use_sz), _encVal(use_octs), _valSZO(0)
  { }

  ~EncoderOfOCTSTR()
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

