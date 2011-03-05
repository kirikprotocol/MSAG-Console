/* ************************************************************************* *
 * BER Decoder: base class of SEQUENCE OF type decoders.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SEQUENCE_OF
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_SEQUENCE_OF

#include "eyeline/asn1/BER/rtdec/ElementDecoderOfSETOF.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeStruct.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************* *
 * Base template class of decoder of SEQUENCE OF with arbitrary
 * data structure representation.
 * ************************************************************* */
template <
    //Must has default and copying constructors
    class _TArg
    //Must has tagged/untagged and copying constructors
  , class _DecoderOfTArg /* : public TypeValueDecoder_T<_TArg>*/
>
class DecoderOfSeqOfAC_T : public DecoderOfStructAC {
protected:
  SETOFElementDecoder _setofDec;
  uint16_t            _maxNum;
  uint16_t            _curNum;
  DecoderProducer_T<_DecoderOfTArg> _pDec;

  //
  void initElementDecoder(const ASTag & fld_tag, ASTagging::Environment_e tag_env,
                          uint16_t max_sz)  /*throw(std::exception)*/
  {
    _pDec.init(fld_tag, tag_env, getTSRule());
    _setofDec.erase();
    _setofDec.setAlternative(0, fld_tag, tag_env, EDAlternative::altOPTIONAL);
    _maxNum = max_sz; _curNum = 0;
  }
  //
  void initElementDecoder(uint16_t max_sz) /*throw(std::exception)*/
  {
    _pDec.init(getTSRule());
    _setofDec.erase();
    if (_pDec->isTagged()) {
      _setofDec.setAlternative(0, *(_pDec->getTag()), EDAlternative::altOPTIONAL);
    } else {
      if (_pDec->getTaggingOptions())
        _setofDec.setAlternative(0, *_pDec->getTaggingOptions(), EDAlternative::altOPTIONAL);
      else { //untagged OpenType
        _setofDec.setAlternative(0, EDAlternative::altOPTIONAL);
      }
    }
    _maxNum = max_sz; _curNum = 0;
  }

  // --------------------------------------------
  // -- DecoderOfSeqOfAC_T<> interface methods
  // --------------------------------------------
  //Allocates structure for next value is to decode, and sets it as
  //value of _DecoderOfTArg.
  virtual void setNextValue(void) /*throw(std::exception)*/ = 0;

  // ------------------------------------------------------
  // -- DecoderOfStructAC interface methods implementation
  // ------------------------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw() */
  {
    if (_maxNum && (_curNum >= _maxNum))
      return NULL;
    //allocate next element and initialize decoder
    setNextValue();
    return _pDec.get();
  }
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { ++_curNum; }

  //constructor for types defined as SEQUENCE OF with own tagging
  DecoderOfSeqOfAC_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, eff_tags, use_rule)
    , _maxNum(0), _curNum(0)
  { }

public:
  // constructor for untagged SEQUENCE OF
  explicit DecoderOfSeqOfAC_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, asn1::_uniTagging().SEQOF, use_rule)
    , _maxNum(0), _curNum(0)
  { }
  // constructor for tagged SEQUENCE OF
  DecoderOfSeqOfAC_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, ASTagging(use_tag, tag_env, asn1::_uniTagging().SEQOF), use_rule)
    , _maxNum(0), _curNum(0)
  { }
  DecoderOfSeqOfAC_T(const DecoderOfSeqOfAC_T & use_obj)
    : DecoderOfStructAC(use_obj)
    , _setofDec(use_obj._setofDec)
    , _maxNum(use_obj._maxNum), _curNum(use_obj._curNum)
  {
    setElementDecoder(_setofDec);
    if (use_obj._pDec.get())
      _pDec.init(*use_obj._pDec.get());
  }
  //
  virtual ~DecoderOfSeqOfAC_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE_OF */

