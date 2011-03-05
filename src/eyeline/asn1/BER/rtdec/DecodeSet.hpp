/* ************************************************************************* *
 * BER Decoder: SET type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SET
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  //virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */ = 0;
  //Performs actions upon successfull optional element decoding
  //virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ = 0;

  //constructor for types defined as SET with own tagging
  explicit DecoderOfSet_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_seqDec, eff_tags, use_rule)
  { }

public:
  // constructor for untagged SET
  explicit DecoderOfSet_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_setDec, asn1::_uniTagging().SETOF, use_rule)
  { }
  // constructor for tagged SET
  DecoderOfSet_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSetAC(_setDec, ASTagging(use_tag, tag_env, asn1::_uniTagging().SETOF), use_rule)
  { }
  //
  explicit DecoderOfSet_T((const DecoderOfSet_T & use_obj)
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

