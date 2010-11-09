/* ************************************************************************* *
 * BER Encoder: NULL type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_NULL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_NULL

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the NULL type value according to X.690
 * clause 8.8.
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 5] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfNULL : public TypeValueEncoderAC {
protected:
  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/;
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                               TSLength max_len) const /*throw(std::exception)*/
  {
    return ENCResult(ENCResult::encOk);
  }
  //
  virtual bool isPortable(TSGroupBER::Rule_e tgt_rule, TSGroupBER::Rule_e curr_rule) const /*throw()*/
  { 
    return true; //NULL has the same encoding for all BER rules.
  }

public:
  //constructor for NULL type
  EncoderOfNULL(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsNULL, use_rule)
  { }

  // constructor for tagged NULL type
  EncoderOfNULL(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsNULL, use_rule)
  { }
  // constructor for encoder of tagged type referencing NULL
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfNULL(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule)
  { }
  //
  ~EncoderOfNULL()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_NULL */

