/* ************************************************************************* *
 * BER Encoder: SEQUENCE OF type encoder base class.
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
class EncoderOfSeqOfAC_T : public EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg> {
protected:
  //Tagged type referencing SEQUENCE OF type encoder constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfSeqOfAC_T(const ASTagging & eff_tags,
                        TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(eff_tags, use_rule)
  { }

public:
  //Untagged SEQUENCE OF type encoder constructor
  explicit EncoderOfSeqOfAC_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(asn1::_tagsSEQOF, use_rule)
  { }
  //Tagged SEQUENCE OF type encoder constructor
  EncoderOfSeqOfAC_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(use_tag, tag_env, asn1::_tagsSEQOF, use_rule)
  { }
  EncoderOfSeqOfAC_T(const EncoderOfSeqOfAC_T & cp_obj)
    : EncoderOfSequenced_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(cp_obj)
  { }
  virtual ~EncoderOfSeqOfAC_T()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  //void setElementTag(const ASTag & elm_tag, ASTagging::Environment_e tag_env) /*throw()*/;
  //void setMaxElements(uint16_t max_elems) /*throw()*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE_OF */

