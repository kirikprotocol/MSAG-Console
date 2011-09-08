/* ************************************************************************* *
 * BER Decoder: SEQUENCE OF type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SEQUENCE_OF_ARRAYED
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_SEQUENCE_OF_ARRAYED

#include "eyeline/asn1/BER/rtdec/DecodeSeqOf.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************* *
 * Template class of decoder of SEQUENCE OF with 'Arrayed'
 * data structure representation.
 * ************************************************************* */
template <
    //Must has default and copying constructors
    class _TArg
    //Must has tagged/untagged and copying constructors
  , class _DecoderOfTArg /* : public TypeValueDecoder_T<_TArg>*/
>
class DecoderOfSeqOfArrayed_T : public DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg> {
public:
  typedef smsc::core::buffers::LWArrayExtension_T<_TArg, uint16_t> ElementsArray;

protected:
  ElementsArray * _dVal;

  // --------------------------------------------
  // -- DecoderOfSeqOfAC_T<> interface methods
  // --------------------------------------------
  //Allocates structure for next value is to decode, and sets it as
  //value of _DecoderOfTArg.
  virtual void setNextValue(void) /*throw(std::exception)*/
  {
    _dVal->reserve(_dVal->size() + 1);
    DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>::_pDec->setValue(_dVal->operator[](_dVal->size()));
  }

  //constructor for types defined as SEQUENCE OF with own tagging
  DecoderOfSeqOfArrayed_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>(eff_tags, use_rule)
    , _dVal(0)
  { }

public:
  // constructor for untagged SEQUENCE OF
  explicit DecoderOfSeqOfArrayed_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>(use_rule)
    , _dVal(0)
  { }
  // constructor for tagged SEQUENCE OF
  DecoderOfSeqOfArrayed_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>(use_tag, tag_env, use_rule)
    , _dVal(0)
  { }
  DecoderOfSeqOfArrayed_T(const DecoderOfSeqOfArrayed_T & use_obj)
    : DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>(use_obj),
    _dVal(use_obj._dVal)
  { }
  //
  virtual ~DecoderOfSeqOfArrayed_T()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  void setValue(ElementsArray & use_arr, uint16_t max_sz = 0)
  {
    _dVal = &use_arr;
    DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>::initElementDecoder(max_sz);
  }
  //
  void setValue(const ASTag & fld_tag, ASTagging::Environment_e tag_env,
                ElementsArray & use_arr, uint16_t max_sz = 0)
  {
    _dVal = &use_arr;
    DecoderOfSeqOfAC_T<_TArg, _DecoderOfTArg>::initElementDecoder(fld_tag, tag_env, max_sz);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE_OF_ARRAYED */

