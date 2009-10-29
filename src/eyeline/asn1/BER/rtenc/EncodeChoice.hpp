#ifndef __ASN1_BER_ENCODER_ENCODECHOICE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENCODECHOICE

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class EncoderOfChoice : public TLVEncoderAC {
protected:
  TSLength        _valSZO;

public:
  // constructor for untagged CHOICE
  EncoderOfChoice(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVEncoderAC(_tagSEQOF, use_rule), _valSZO(0)
  { }
  // constructor for tagged CHOICE
  EncoderOfChoice(const ASTagging & use_tags,
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

  void setSelection(const TLVEncoderAC* used_field);
};

}}}

#endif
