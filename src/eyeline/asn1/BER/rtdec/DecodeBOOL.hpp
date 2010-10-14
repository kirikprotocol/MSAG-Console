/* ************************************************************************* *
 * BER Decoder: BOOLEAN type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_BOOL
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_BOOL

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the BOOLEAN value according to X.690
 * clause 8.2 (with appropriate DER/CER restrctions).
 * ************************************************************************* */
class DecoderOfBOOL : public TypeValueDecoderAC {
private:
  bool * _dVal;

protected:
  // -- ------------------------------------------------- --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ------------------------------------------------- --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::exception)*/;

  //Tagged type referencing BOOLEAN.
  // NOTE: eff_tags is a complete effective tagging of type!
  DecoderOfBOOL(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule)
    , _dVal(0)
  { }

public:
  //Untagged BOOLEAN type encoder constructor. 
  explicit DecoderOfBOOL(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(asn1::_tagsBOOL, use_rule)
    , _dVal(0)
  { }
  //Tagged BOOLEAN type encoder constructor
  DecoderOfBOOL(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(use_tag, tag_env, asn1::_tagsBOOL, use_rule)
    , _dVal(0)
  { }
  ~DecoderOfBOOL()
  { }

  void setValue(bool & use_val) { _dVal = &use_val; }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_BOOL */

