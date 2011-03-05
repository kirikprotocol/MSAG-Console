/* ************************************************************************* *
 * BER Encoder: SEQUENCE OF type encoder (arrayed representation).
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCE_OF_ARRAYED
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_SEQUENCE_OF_ARRAYED

#include "eyeline/util/LWArray.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequenced.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************* *
 * Template class of encoder of SEQUENCE OF with 'Arrayed'
 * data structure representation.
 * ************************************************************* */
template <
    //Must has default and copying constructors
    class _TArg
    //Must has tagged/untagged and copying constructors
  , class _EncoderOfTArg /* : public TypeValueEncoder_T<_TArg>*/
  , uint16_t _NumElemsTArg = 2 //estimated number of element values
>
class EncoderOfSeqOfArrayed_T : public EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg> {
protected:
  //constructor for types defined as SEQUENCE OF with own tagging
  explicit EncoderOfSeqOfArrayed_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(eff_tags, use_rule)
  { }

public:
  typedef eyeline::util::LWArrayExtension_T<_TArg, uint16_t> ElementsArray;
  typedef eyeline::util::LWArrayExtension_T<_TArg *, uint16_t> ElementsPtrArray;

  // constructor for untagged SEQUENCE OF
  explicit EncoderOfSeqOfArrayed_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(asn1::_uniTagging().SEQOF, use_rule)
  { }
  // constructor for tagged SEQUENCE OF
  EncoderOfSeqOfArrayed_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg, _NumElemsTArg>(use_tag, tag_env, asn1::_uniTagging().SEQOF, use_rule)
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  //void setElementTag(const ASTag & elm_tag, ASTagging::Environment_e tag_env) /*throw()*/;
  //void setMaxElements(uint16_t max_elems) /*throw()*/;

  void setValue(const ElementsArray & use_arr) /*throw(std::exception)*/
  {
    uint16_t last_idx = 0;
    if (!use_arr.empty()) {
      reserveElementEncoders(use_arr.size()); //throws
      for(; last_idx < use_arr.size(); ++last_idx)
        addElementValue(use_arr.at(last_idx)); //throws
    }
    this->clearElements(last_idx);
  }
  //
  void setValue(const ElementsPtrArray & use_arr) /*throw(std::exception)*/
  {
    uint16_t last_idx = 0;
    if (!use_arr.empty()) {
      reserveElementEncoders(use_arr.size()); //throws
      for(uint16_t i = 0; i < use_arr.size(); ++i) {
        if (use_arr.at(i)) {
          addElementValue(use_arr.at(i)); //throws
          ++last_Idx;
        }
      }
    }
    this->clearElements(last_idx);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCE_OF_ARRAYED */

