/* ************************************************************************* *
 * BER Decoder: OBJECT-IDENTIFIER type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_OID
#ident "@(#)$Id: "
#define __ASN1_BER_DECODER_OID

#include "eyeline/asn1/ObjectID.hpp"
#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER/DER/CER the OBJECT-IDENTIFIER value according to X.690
 * clause 8.19 (with appropriate DER/CER restrictions).
 * ************************************************************************* */
class DecoderOfObjectID : public TypeValueDecoderAC {
private:
  asn1::ObjectID * _dVal;

protected:
  // -- ------------------------------------------------- --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ------------------------------------------------- --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::exception)*/;

  //Tagged type referencing OBJECT-IDENTIFIER. 
  // NOTE: eff_tags is a complete effective tagging of type!
  DecoderOfObjectID(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule)
    , _dVal(0)
  { }

public:
  //Untagged OBJECT-IDENTIFIER type encoder constructor. 
  explicit DecoderOfObjectID(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(asn1::_tagsObjectID, use_rule)
    , _dVal(0)
  { }
  //Tagged OBJECT-IDENTIFIER type encoder constructor
  DecoderOfObjectID(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(use_tag, tag_env, asn1::_tagsObjectID, use_rule)
    , _dVal(0)
  { }
  ~DecoderOfObjectID()
  { }

  void setValue(asn1::ObjectID & use_val) { _dVal = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_OID */

