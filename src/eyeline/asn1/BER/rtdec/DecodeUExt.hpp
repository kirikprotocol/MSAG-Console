/* ************************************************************************* *
 * BER Decoder: decoder of unknown extension of structured type.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_UNKNOWN_EXTENSION
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_UNKNOWN_EXTENSION

#include <list>
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

typedef std::list<TransferSyntax> UnkExtensionList;

class DecoderOfUExtension : public TypeValueDecoderAC {
protected:
  UnkExtensionList * _valDec;

  DECResult decodeVAL(const TLVProperty * val_prop,
                      const uint8_t * use_enc, TSLength max_len,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                      bool relaxed_rule = false)
    /*throw(BERDecoderException)*/;

public:
  DecoderOfUExtension(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(0, use_rule), _valDec(0)
  { }
  ~DecoderOfUExtension()
  { }

  void setValue(UnkExtensionList & use_val) { _valDec = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_UNKNOWN_EXTENSION */
