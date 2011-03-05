/* ************************************************************************* *
 * BER Decoder: ObjectDescriptor type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_OBJDESCRIPTOR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_OBJDESCRIPTOR

#include "eyeline/asn1/BER/rtdec/DecodeGraphSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER/DER/CER the ObjectDescriptor value encoding according to X.690
 * clause 8.21 (with appropriate DER/CER restrctions).
 *
 * NOTE: ObjectDescriptor is defined as [UNIVERSAL 7] IMPLICIT GraphicString
 * ************************************************************************* */
class DecoderOfObjDescriptor : public RCSTRValueDecoder {
protected:
  GraphStrDecConverter  _vCvt;

public:
  //Constructor for base type
  DecoderOfObjDescriptor(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, asn1::_uniTagging().ObjDescriptor, use_rule)
  { }
  //Constructor for tagged base type
  DecoderOfObjDescriptor(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : RCSTRValueDecoder(_vCvt, ASTagging(use_tag, tag_env, asn1::_uniTagging().ObjDescriptor), use_rule)
  { }
  virtual ~DecoderOfObjDescriptor()
  { }

  void setValue(std::string & use_val) { _vCvt.setValue(use_val); }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_OBJDESCRIPTOR */

