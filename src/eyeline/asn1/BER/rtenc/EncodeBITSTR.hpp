/* ************************************************************************* *
 * BER Encoder: BIT STRING type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_BITSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_BITSTR

#include "eyeline/asn1/BITSTR.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the BIT STRING value according to X.690
 * clause 8.6 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 3] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfBITSTR : public TypeValueEncoderAC {
protected:
  TSLength        _encValBits;
  const uint8_t * _encVal;
  TSLength        _encValOcts;

  //Returns number of unused bits in last octet
  uint8_t unusedBits(void) const { return (uint8_t)(8U - _encValBits%8); }

  TSLength numFragments(void) const
  { //NOTE: one byte of 1000 bytes fragment is taken by unused bits marker
    return (_encValOcts/999 + (_encValOcts%999 ? 1 : 0));
  }

  //Returns total length of fragmented encoding
  //Throws if value is too large and cann't be encoded
  TSLength calculateFragments(void) const /*throw(std::exception)*/;

  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/;
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                              TSLength max_len) const /*throw(std::exception)*/;


  //Constructors for encoder of tagged type referencing BIT STRING
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfBITSTR(const ASTagging & use_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tags, use_rule)
    , _encValBits(0), _encVal(0), _encValOcts(0)
  { }

public:
  //Constructors for encoder of BIT STRING type
  EncoderOfBITSTR(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsBITSTR, use_rule)
    , _encValBits(0), _encVal(0), _encValOcts(0)
  { }
  EncoderOfBITSTR(TSLength num_bits, const uint8_t * use_octs,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsBITSTR, use_rule)
    , _encValBits(num_bits), _encVal(use_octs)
    , _encValOcts(eyeline::util::bitsNum2Octs(_encValBits))
  { }
  //Constructors for encoder of tagged BIT STRING type
  EncoderOfBITSTR(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsBITSTR, use_rule)
    , _encValBits(0), _encVal(0), _encValOcts(0)
  { }
  EncoderOfBITSTR(TSLength num_bits, const uint8_t * use_octs,
                  const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsBITSTR, use_rule)
    , _encValBits(num_bits), _encVal(use_octs)
    , _encValOcts(eyeline::util::bitsNum2Octs(_encValBits))
  { }
  //
  ~EncoderOfBITSTR()
  { }

  void setValue(const asn1::BITArrayTiny & use_val) /*throw(std::exception)*/
  {
    _encValBits = (TSLength)use_val.size();
    _encVal = use_val.getOcts();
    _encValOcts = eyeline::util::bitsNum2Octs(_encValBits);
  }
  void setValue(const asn1::BITArray64k & use_val) /*throw(std::exception)*/
  {
    _encValBits = (TSLength)use_val.size();
    _encVal = use_val.getOcts();
    _encValOcts = eyeline::util::bitsNum2Octs(_encValBits);
  }
  void setValue(TSLength num_bits, const uint8_t * use_octs) /*throw(std::exception)*/
  {
    _encValBits = num_bits;
    _encVal = use_octs;
    _encValOcts = eyeline::util::bitsNum2Octs(_encValBits);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_BITSTR */

