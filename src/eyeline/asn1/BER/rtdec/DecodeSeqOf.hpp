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
private:
  union {
    void * _aligner;
    uint8_t _buf[sizeof(_DecoderOfTArg)];
  } _memDec;

  void allcValDecoder(void)
  {
    if (_pDec)
      _pDec->~_DecoderOfTArg();
    _pDec = new (_memDec._buf) _DecoderOfTArg();
    _pDec->setTSRule(getTSRule());
  }
  void allcValDecoder(const ASTag & fld_tag, ASTagging::Environment_e tag_env)
  {
    if (_pDec)
      _pDec->~_DecoderOfTArg();
    _pDec = new (_memDec._buf) _DecoderOfTArg(fld_tag, tag_env);
    _pDec->setTSRule(getTSRule());
  }
  
  void allcValDecoder(const _DecoderOfTArg & use_obj)
  {
    if (_pDec)
      _pDec->~_DecoderOfTArg();
    _pDec = new (_memDec._buf) _DecoderOfTArg(use_obj);
    _pDec->setTSRule(getTSRule());
  }

protected:
  SETOFElementDecoder _setofDec;
  _DecoderOfTArg *    _pDec;
  uint16_t            _maxNum;
  uint16_t            _curNum;

  //
  void initElementDecoder(const ASTag & fld_tag, ASTagging::Environment_e tag_env,
                          uint16_t max_sz)
    /*throw(std::exception)*/
  {
    allcValDecoder(fld_tag, tag_env);
    _setofDec.erase();
    _setofDec.setAlternative(0, fld_tag, tag_env, EDAlternative::altOPTIONAL);
    _maxNum = max_sz; _curNum = 0;
  }
  //
  void initElementDecoder(uint16_t max_sz) /*throw(std::exception)*/
  {
    allcValDecoder();
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
    return _pDec;
  }
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { ++_curNum; }

  //constructor for types defined as SEQUENCE OF with own tagging
  DecoderOfSeqOfAC_T(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, eff_tags, use_rule)
    , _pDec(0), _maxNum(0), _curNum(0)
  { }

public:
  // constructor for untagged SEQUENCE OF
  explicit DecoderOfSeqOfAC_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, asn1::_tagsSEQOF, use_rule)
    , _pDec(0), _maxNum(0), _curNum(0)
  { }
  // constructor for tagged SEQUENCE OF
  DecoderOfSeqOfAC_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfStructAC(_setofDec, ASTagging(use_tag, tag_env, asn1::_tagsSEQOF), use_rule)
    , _pDec(0), _maxNum(0), _curNum(0)
  { }
  DecoderOfSeqOfAC_T(const DecoderOfSeqOfAC_T & use_obj)
    : DecoderOfStructAC(use_obj)
    , _setofDec(use_obj._setofDec)
    , _pDec(0), _maxNum(use_obj._maxNum), _curNum(use_obj._curNum)
  {
    setElementDecoder(_setofDec);
    allcValDecoder(*use_obj._pDec);
  }
  //
  virtual ~DecoderOfSeqOfAC_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SEQUENCE_OF */

