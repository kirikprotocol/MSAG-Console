/* ************************************************************************* *
 * BER Decoder: decoder of unknown extension of structured type.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_UNKNOWN_EXTENSION
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_UNKNOWN_EXTENSION

#include "eyeline/asn1/UnknownExtensions.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//UnknownExtension is encoded as untagged Opentype
class DecoderOfUExtension : public DecoderOfASType {
public:
  explicit DecoderOfUExtension(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfASType(use_rule)
  { }
  ~DecoderOfUExtension()
  { }

  void setValue(asn1::UnknownExtensions & use_val)
  {
    use_val._tsList.push_back(TransferSyntax());
    DecoderOfASType::setValue(use_val._tsList.back());
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_UNKNOWN_EXTENSION */

