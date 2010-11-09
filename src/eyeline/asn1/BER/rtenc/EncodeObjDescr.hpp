/* ************************************************************************* *
 * BER Encoder: ObjectDescriptor type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_OBJDESCRIPTOR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_OBJDESCRIPTOR

#include "eyeline/asn1/BER/rtenc/EncodeGraphStr.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the ObjectDescriptor value according to X.690
 * clause 8.21 (with appropriate DER/CER restrctions).
 *
 * NOTE: ObjectDescriptor is defined as [UNIVERSAL 7] IMPLICIT GraphicString
 * ************************************************************************* */
class EncoderOfObjDescriptor : public EncoderOfRCSTR {
protected:
  GraphSTREncConverter  _valConv;

public:
  //Constructors for ObjectDescriptor type
  EncoderOfObjDescriptor(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, asn1::_tagsObjDescriptor, use_rule)
    , _valConv(0, 0)
  { }
  //Constructors for ObjectDescriptor type
  EncoderOfObjDescriptor(TSLength str_sz, const char * use_chars,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, asn1::_tagsObjDescriptor, use_rule)
    , _valConv(str_sz, use_chars)
  { }

  //Constructors for tagged ObjectDescriptor type
  EncoderOfObjDescriptor(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, use_tag, tag_env, asn1::_tagsObjDescriptor, use_rule)
    , _valConv(0, 0)
  { }
  EncoderOfObjDescriptor(TSLength str_sz, const char * use_chars,
                        const ASTag & use_tag, ASTagging::Environment_e tag_env,
                        TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, use_tag, tag_env, asn1::_tagsObjDescriptor, use_rule)
    , _valConv(str_sz, use_chars)
  { }
  //
  ~EncoderOfObjDescriptor()
  { }

  void setValue(TSLength str_sz, const char * use_chars)
  {
    _valConv.init(str_sz, use_chars);
  }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_OBJDESCRIPTOR */

