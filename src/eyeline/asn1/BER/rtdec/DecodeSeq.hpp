/* ************************************************************************* *
 * BER Decoder: SEQUENCE type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SEQUENCE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_SEQUENCE

#include "eyeline/asn1/BER/rtdec/ElementDecoderOfSEQ.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeStruct.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//NOTE: altering of _NumOfMandatoryArg is meaningfull only if number of
//mandatory fields much lesser than number of optional fields.
template <uint16_t _SizeTArg, uint16_t _NumOfMandatoryArg = _SizeTArg>
class DecoderOfSequence_T : public DecoderOfStructAC {
protected:
  SEQElementDecoder_T<_SizeTArg, _NumOfMandatoryArg>  _seqDec;
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  //virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */ = 0;
  //Performs actions upon successfull optional element decoding
  //virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ = 0;

  //constructor for types defined as SEQUENCE with own tagging
  explicit DecoderOfSequence_T(const ASTagging & eff_tags,
                               TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_seqDec, eff_tags, use_rule)
  { }

public:
  // constructor for untagged SEQUENCE
  explicit DecoderOfSequence_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_seqDec, asn1::_tagsSEQOF, use_rule)
  { }
  // constructor for tagged SEQUENCE
  DecoderOfSequence_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_seqDec, ASTagging(use_tag, tag_env, asn1::_tagsSEQOF), use_rule)
  { }
  //copying constructor
  DecoderOfSequence_T(const DecoderOfSequence_T & use_obj)
    : DecoderOfStructAC(use_obj)
  {
    setElementDecoder(_seqDec);
  }
  //
  ~DecoderOfSequence_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE */

