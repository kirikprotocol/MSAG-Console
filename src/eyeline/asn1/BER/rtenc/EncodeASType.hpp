/* ************************************************************************* *
 * BER Encoder: ANY and ABSTRACT-SYNTAX.&Type (Opentype) encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ASTYPE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ASTYPE

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class EncoderOfASType : public TypeValueEncoderAC {
private:
  TaggingOptions  _altTags; //taggings of Opentype alternatives

  //following members meaningful in case of OpenType value is a TransferSyntax
  TLVProperty     _vProp;
  const uint8_t * _valTS;

protected:
  void setSelection(const ASTagging & use_tags, ValueEncoderIface * val_enc)
    /*throw(std::exception)*/;

  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/
  {
    val_prop = _vProp;
  }
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                              TSLength max_len) const /*throw(std::exception)*/;

  // constructor for encoder of tagged type referencing OpenType
  EncoderOfASType(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, _altTags, use_rule), _valTS(0)
  { }

public:
  // constructor for encoder of untagged OpenType
  EncoderOfASType(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(_altTags, use_rule), _valTS(0)
  { }
  // constructor for encoder of tagged OpenType
  EncoderOfASType(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, _altTags, use_rule), _valTS(0)
  { }
  ~EncoderOfASType()
  { }

  //
  void setValue(TypeEncoderAC & use_val) /*throw(std::exception)*/;
  //
  void setValue(const TransferSyntax & use_ts_enc) /*throw(std::exception)*/;
};

typedef EncoderOfASType EncoderOfANY;

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ASTYPE */

