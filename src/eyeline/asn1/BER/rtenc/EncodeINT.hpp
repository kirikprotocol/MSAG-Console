/* ************************************************************************* *
 * BER Encoder: INTEGER type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_INTEGER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_INTEGER

#include <inttypes.h>
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the INTEGER value according to X.690
 * clause 8.3 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 2] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfINTEGER : public TypeValueEncoderAC {
private:
private:
  enum IntSZO_e { szoNone = 0, szo8 = 1, szo16 = 2, szo32 = 4 };

  IntSZO_e  _vSzo;
  bool      _signed;
  union {
    int8_t    i8;
    uint8_t   u8;
    int16_t   i16;
    uint16_t  u16;
    int32_t   i32;
    uint32_t  u32;
  } _encVal;          //value is to encode, negative number is 
                      //converted to 'two's complement' form
protected:
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
    return true; //INTEGER has the same encoding for all BER rules.
  }

public:
  //Constructor for INTEGER type
  EncoderOfINTEGER(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsINTEGER, use_rule)
    , _vSzo(szoNone), _signed(false)
  {
    _encVal.u32 = 0;
  }
  EncoderOfINTEGER(const uint32_t & use_val,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(asn1::_tagsINTEGER, use_rule)
    , _vSzo(szo32), _signed(false)
  {
    setValue(use_val);
  }
  //Constructor for tagged INTEGER type
  EncoderOfINTEGER(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsINTEGER, use_rule)
    , _vSzo(szoNone), _signed(false)
  {
    _encVal.u32 = 0;
  }
  EncoderOfINTEGER(const uint32_t & use_val,
                   const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, asn1::_tagsINTEGER, use_rule)
    , _vSzo(szo32), _signed(false)
  {
    setValue(use_val);
  }
  // constructor for encoder of tagged type referencing INTEGER
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfINTEGER(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule), _vSzo(szoNone)
  {
    _encVal.u32 = 0; 
  }
  //
  ~EncoderOfINTEGER()
  { }

  void setValue(const uint8_t & use_val)
  {
    _vSzo = szo8; _encVal.u8 = use_val; _signed = false;
  }
  void setValue(const int8_t & use_val)
  {
    _vSzo = szo8; _encVal.i8 = use_val; _signed = true;
  }

  void setValue(const uint16_t & use_val)
  {
    _vSzo = szo16; _encVal.u16 = use_val; _signed = false;
  }
  void setValue(const int16_t & use_val)
  {
    _vSzo = szo16; _encVal.i16 = use_val; _signed = true;
  }

  void setValue(const uint32_t & use_val)
  {
    _vSzo = szo32; _encVal.u32 = use_val; _signed = false;
  }
  void setValue(const int32_t & use_val)
  {
    _vSzo = szo32; _encVal.i32 = use_val; _signed = true;
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_INTEGER */

