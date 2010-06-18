/* ************************************************************************* *
 * BER Decoder: SET type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SET
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_SET

#include "eyeline/asn1/BER/rtdec/ElementDecoderOfSET.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeStruct.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <uint16_t _SizeTArg>
class DecoderOfSet_T : public DecoderOfStructAC {
private:
  SETElementDecoder_T<_SizeTArg>  _setDec;

protected:
  //constructor for types defined as SET with own tagging
  DecoderOfSet_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_seqDec, eff_tags, use_rule)
  { }

public:
  // constructor for untagged SET
  DecoderOfSet_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_setDec, asn1::_tagsSETOF, use_rule)
  { }
  // constructor for tagged SET
  DecoderOfSet_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_setDec, ASTagging(use_tag, tag_env, asn1::_tagsSETOF), use_rule)
  { }
  //
  DecoderOfSet_T((const DecoderOfSet_T & use_obj)
    : DecoderOfSetAC(use_obj)
  {
    setElementDecoder(_setDec);
  }
  //
  ~DecoderOfSet_T()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SET */

