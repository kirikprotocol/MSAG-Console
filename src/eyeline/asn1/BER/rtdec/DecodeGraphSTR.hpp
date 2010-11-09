/* ************************************************************************* *
 * BER Decoder: GraphicString type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_GRAPHSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_GRAPHSTR

#include <string>
#include "eyeline/asn1/BER/rtdec/DecodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class GraphStrDecConverter : public RCSDecConverterIface {
protected:
  std::string * _dVal;

public:
  explicit GraphStrDecConverter(std::string * use_val = 0)
    : _dVal(use_val)
  { }
  ~GraphStrDecConverter()
  { }

  void setValue(std::string & use_val) { _dVal = &use_val; }

  // -------------------------------------------
  // RCSDecConverterIface implementation
  // -------------------------------------------
  //Returns universal tag of fragment in case of fragmented string encoding
  const ASTag & fragmentTag(void) const /* throw()*/ { return _tagOCTSTR; }
  //Incrementally converts next fragment of string encoding to
  //characters from associated alphabet.
  //Returns number of bytes consumed from buffer.
  DECResult unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/;
};


/* ************************************************************************* *
 * Decodes by BER/DER/CER the GraphicString value encoding according to X.690
 * clause 8.21 (with appropriate DER/CER restrctions).
 *
 * NOTE: GraphicString is defined as [UNIVERSAL 25] IMPLICIT OCTET STRING
 * ************************************************************************* */
//UNITypeValueDecoder_T<std::string, asn1::_tagsGraphicSTR>
class DecoderOfGraphicSTR : public RCSTRValueDecoder {
protected:
  GraphStrDecConverter  _vCvt;

public:
  //Constructor for base type
  explicit DecoderOfGraphicSTR(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, asn1::_tagsGraphicSTR, use_rule)
  { }
  //Constructor for tagged base type
  DecoderOfGraphicSTR(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                   TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, ASTagging(use_tag, tag_env, asn1::_tagsGraphicSTR), use_rule)
  { }
  virtual ~DecoderOfGraphicSTR()
  { }

  void setValue(std::string & use_val) { _vCvt.setValue(use_val); }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_GRAPHSTR */

