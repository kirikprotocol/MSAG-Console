/* ************************************************************************* *
 * BER Encoder: SEQUENCE OF type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCE_OF
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_SEQUENCE_OF

#include "eyeline/asn1/BER/rtenc/EncodeSequenced.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  class _TArg
  //must have tagged & untagged type constructors
  , class _EncoderOfTArg /*: public TypeValueEncoder_T<_TArg>*/ 
  , uint16_t _NumElemsTArg //estimated number of element values
>
class EncoderOfSequenceOf_T : public EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg> {
protected:
  //Tagged type referencing SEQUENCE OF type encoder constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfSequenceOf_T(const ASTagging & eff_tags,
                        TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_EncoderOfTArg, _NumElemsTArg>(eff_tags, use_rule)
  { }

public:
  //Untagged SEQUENCE OF type encoder constructor
  explicit EncoderOfSequenceOf_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_EncoderOfTArg, _NumElemsTArg>(asn1::_tagsSEQOF, use_rule)
  { }
  //Tagged SEQUENCE OF type encoder constructor
  EncoderOfSequenced_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_EncoderOfTArg, _NumElemsTArg>(use_tag, tag_env, asn1::_tagsSEQOF, use_rule)
  { }
  virtual ~EncoderOfSequenced_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE_OF */

