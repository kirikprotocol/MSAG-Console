/* ************************************************************************* *
 * BER Encoder: GraphicString type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_GRAPHIC_STR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_GRAPHIC_STR

#include "eyeline/asn1/BER/rtenc/EncodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Converts value of GraphicString type to its octet string
 * representation performing necessary character transformations (escaping, etc)
 * ************************************************************************* */
class GraphSTREncConverter : public RCSEncConverterIface {
protected:
  TSLength      _encValSz;
  const char *  _encVal;
  TSLength      _numConverted;

public:
  GraphSTREncConverter(TSLength use_sz = 0, const char * use_chars = 0)
    : _encValSz(use_sz), _encVal(use_chars), _numConverted(0)
  { }

  void init(TSLength use_sz, const char * use_chars)
  {
    _encValSz = use_sz; _encVal = use_chars; _numConverted = 0;
  }

  //Returns length of resulted (converted) encoding of string
  TSLength getPackedLength(void)
  { //NOTE: control and escaping sequences affect only visual
    //      representation of characters. 
    return _encValSz;
  }
  //Incrementally converts next portion of string value writing
  //requested number of bytes to provided buffer.
  //Returns number of bytes written to buffer.
  TSLength pack2Octs(uint8_t * use_buf, TSLength req_bytes);
};


/* ************************************************************************* *
 * Encodes by BER/DER/CER the GraphicString value according to X.690
 * clause 8.21 (with appropriate DER/CER restrctions).
 *
 * NOTE: GraphicString is defined as [UNIVERSAL 25] IMPLICIT OCTET STRING
 * ************************************************************************* */
class EncoderOfGraphicSTR : public EncoderOfRCSTR {
protected:
  GraphSTREncConverter _valConv;

  // constructor for encoder of tagged type referencing GraphicString
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfGraphicSTR(const ASTagging & eff_tags,
                      TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, eff_tags, use_rule)
    , _valConv(0, 0)
  { }

public:
  //Constructors for GraphicString type
  EncoderOfGraphicSTR(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, asn1::_tagsGraphicSTR, use_rule)
    , _valConv(0, 0)
  { }
  //Constructors for tagged GraphicString type
  EncoderOfGraphicSTR(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                      TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, use_tag, tag_env, asn1::_tagsGraphicSTR, use_rule)
    , _valConv(0, 0)
  { }
  //
  ~EncoderOfGraphicSTR()
  { }

  void setValue(TSLength str_sz, const char * use_chars)
  {
    _valConv.init(str_sz, use_chars);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_GRAPHIC_STR */

