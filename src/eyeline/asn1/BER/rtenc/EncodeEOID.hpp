/* ************************************************************************* *
 * BER Encoder: OBJECT-IDENTIFIER type encoder, operates with encoded
 *              form of OID value.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_EOID
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_EOID

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::EncodedOID;

/* ************************************************************************* *
 * Encodes by BER/DER/CER the OBJECT-IDENTIFIER value (encoded form)
 * according to X.690 clause 8.19 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 6] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfEOID : public TypeValueEncoderAC {
protected:
  const EncodedOID * _encVal;

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
    return true; //OBJECT-IDENTIFIER has the same encoding for all BER rules.
  }

  //Constructor for encoder of tagged OBJECT IDENTIFIER type
  // NOTE: eff_tags must be a complete tagging of type!
  EncoderOfEOID(const ASTagging & eff_tags,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule), _encVal(0)
  { }

public:
  //Constructor for encoder of OBJECT IDENTIFIER type
  EncoderOfEOID(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsObjectID, use_rule)
    , _encVal(0)
  { }
  EncoderOfEOID(const EncodedOID & use_val,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsObjectID, use_rule)
    , _encVal(&use_val)
  { }

  //Constructor for encoder of tagged OBJECT IDENTIFIER type
  EncoderOfEOID(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsObjectID, use_rule)
    , _encVal(0)
  { }
  EncoderOfEOID(const EncodedOID & use_val,
                const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsObjectID, use_rule)
    , _encVal(&use_val)
  { }
  ~EncoderOfEOID()
  { }

  // ----------------------------------------------------------------------------- //
  // -- NOTE: type tagging MUST be set prior to referenced value assignment     -- //
  // ----------------------------------------------------------------------------- //
  void setValue(const EncodedOID & use_val) { _encVal = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_EOID */

