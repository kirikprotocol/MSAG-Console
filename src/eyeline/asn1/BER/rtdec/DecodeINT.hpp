/* ************************************************************************* *
 * BER Decoder: INTEGER type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_INTEGER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_INTEGER

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the INTEGER value according to X.690
 * clause 8.3 (with appropriate DER/CER restrctions).
 * ************************************************************************* */
class DecoderOfINTEGER : public TypeValueDecoderAC {
private:
  enum IntSZO_e { szoNone = 0, szo8 = 1, szo16 = 2, szo32 = 4 };

  IntSZO_e  _vSzo;
  bool      _vSigned;
  union {
    int8_t  *   i8;
    uint8_t *   u8;
    int16_t *   i16;
    uint16_t *  u16;
    int32_t  *  i32;
    uint32_t *  u32;
  } _pVal;

protected:
  // -- ------------------------------------------------- --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ------------------------------------------------- --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(std::exception)*/;

  //Tagged type referencing INTEGER. 
  // NOTE: eff_tags is a complete effective tagging of type!
  explicit DecoderOfINTEGER(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, use_rule)
    , _vSzo(szoNone), _vSigned(false)
  {
    _pVal.u32 = 0;
  }

public:
  //Untagged INTEGER type encoder constructor. 
  explicit DecoderOfINTEGER(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(asn1::_tagsINTEGER, use_rule)
    , _vSzo(szoNone), _vSigned(false)
  {
    _pVal.u32 = 0;
  }
  //Tagged INTEGER type encoder constructor
  DecoderOfINTEGER(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                     TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(use_tag, tag_env, asn1::_tagsINTEGER, use_rule)
    , _vSzo(szoNone), _vSigned(false)
  {
    _pVal.u32 = 0;
  }
  ~DecoderOfINTEGER()
  { }

  void setValue(uint8_t & use_val)
  {
    _vSigned = false; _vSzo = szo8; _pVal.u8 = &use_val; 
  }
  void setValue(int8_t & use_val)
  {
    _vSigned = true; _vSzo = szo8; _pVal.i8 = &use_val; 
  }
  void setValue(uint16_t & use_val)
  {
    _vSigned = false; _vSzo = szo16; _pVal.u16 = &use_val;
  }
  void setValue(int16_t & use_val)
  {
    _vSigned = true; _vSzo = szo16; _pVal.i16 = &use_val;
  }
  void setValue(uint32_t & use_val)
  {
    _vSigned = false; _vSzo = szo32; _pVal.u32 = &use_val;
  }
  void setValue(int32_t & use_val)
  {
    _vSigned = true; _vSzo = szo32; _pVal.i32 = &use_val;
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_INTEGER */

