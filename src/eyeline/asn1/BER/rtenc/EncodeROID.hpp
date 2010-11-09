/* ************************************************************************* *
 * BER Encoder: RELATIVE-OID type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ROID
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_ROID

#include "eyeline/asn1/ObjectID.hpp"
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
class EncoderOfRelativeOID : public TypeValueEncoderAC {
protected:
  const asn1::RelativeOID * _encVal;

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
    return true; //RELATIVE-OID has the same encoding for all BER rules.
  }

  // constructor for encoder of tagged type referencing RELATIVE-OID 
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfRelativeOID(const ASTagging & eff_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule), _encVal(0)
  { }

public:
  //Constructor for RELATIVE-OID type
  EncoderOfRelativeOID(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsRelativeOID, use_rule), _encVal(0)
  { }
  EncoderOfRelativeOID(const asn1::RelativeOID & use_val,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsRelativeOID, use_rule), _encVal(&use_val)
  { }
  //Constructor for tagged RELATIVE-OID type
  EncoderOfRelativeOID(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsRelativeOID, use_rule), _encVal(0)
  { }
  EncoderOfRelativeOID(const asn1::RelativeOID & use_val,
                       const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsRelativeOID, use_rule), _encVal(&use_val)
  { }
  ~EncoderOfRelativeOID()
  { }

  void setValue(const asn1::RelativeOID & use_val) { _encVal = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ROID */

