/* ************************************************************************* *
 * BER Encoder: BOOLEAN type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_BOOL
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_BOOL

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the BOOLEAN type value according to X.690
 * clause 8.2.
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 1] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfBOOL : public TypeValueEncoderAC {
protected:
  bool  _encVal;

  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/;
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                               TSLength max_len) const /*throw(std::exception)*/;
  //
  virtual bool isPortable(TSGroupBER::Rule_e tgt_rule, TSGroupBER::Rule_e curr_rule) const /*throw()*/
  { 
    return true; //BOOLEAN has the same encoding for all BER rules.
  }

  // constructor for encoder of tagged type referencing BOOL
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfBOOL(const ASTagging & eff_tags,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule)
  { }

public:
  //constructor for encoder of BOOLEAN type 
  EncoderOfBOOL(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsBOOL, use_rule)
  { }
  // constructor for encoder of tagged BOOLEAN type
  EncoderOfBOOL(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsBOOL, use_rule)
  { }
  ~EncoderOfBOOL()
  { }
  
  void setValue(bool use_val) { _encVal = use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_BOOL */

