#ifndef __ASN1_BER_ENCODER_ENCODESEQUENCE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENCODESEQUENCE

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/INTEGER.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the RELATIVE-OID value according to X.690
 * clause 8.20 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 13] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfSequence : public TLVEncoderAC {
protected:
  TSLength        _valSZO;

public:
  EncoderOfSequence(const ASTag * use_tag = NULL,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVEncoderAC((use_tag ? *use_tag : _tagSEQOF), use_rule), _valSZO(0)
  { }
  EncoderOfSequence(const ASTagging & use_tags,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVEncoderAC(use_tags, use_rule), _valSZO(0)
  { }

  //Calculates length of BER/DER/CER encoding of type value.
  //If 'do_indef' flag is set, then length of value encoding is computed
  //even if TLVLayout uses only indefinite form of length determinants.
  //NOTE: may change TLVLayout if type value encoding should be fragmented.
  //NOTE: Throws in case of value that cann't be encoded.
  TSLength  calculateVAL(bool do_indef = false) /*throw(std::exception)*/;

  //Encodes by BER/DER/CER the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const;

  void addField(const TLVEncoderAC* used_field);
};

} //ber
} //asn1
} //eyeline

#endif
