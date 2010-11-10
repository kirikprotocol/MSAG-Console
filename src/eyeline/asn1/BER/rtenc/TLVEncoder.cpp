#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/TagEncoder.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeLDs_impl.hpp"


namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class TLComposer implementation:
 * ************************************************************************* */
void TLComposer::calculate(const ASTag & use_tag)
{
  //here encode_tag_internal() cann't fail, otherwise assertion!
  _szoTag = compose_toc(use_tag, _isConstructed, _octTag, (uint8_t)sizeof(_octTag));
  if (isDefinite()) {
    //here encode_identifier() cann't fail, otherwise assertion!
    _szoLOC = encode_ldeterminant(_valLen, _octLOC, (uint8_t)sizeof(_octLOC));
  } else {
    _octLOC[0] = 0x80;
    _szoLOC = 1;
  }
}

//Encodes 'begin-of-content' octets of TLV encoding
ENCResult TLComposer::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (max_len < getBOCsize()) {
    rval.status = ENCResult::encMoreMem;
  } else {
    //copy Tag encoding
    for (uint8_t i = 0; i < _szoTag; ++i)
      use_enc[i] = _octTag[i];
    //copy LD encoding
    for (uint8_t i = 0; i < _szoLOC; ++i)
      use_enc[_szoTag + i] = _octLOC[i];
    rval.nbytes = _szoTag + _szoLOC;
  }
  return rval;
}

//Encodes 'end-of-content' octets of TLV encoding
ENCResult TLComposer::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (getEOCsize()) { //indefinite form of LD
    if (max_len < getEOCsize()) {
      rval.status = ENCResult::encMoreMem;
    } else {
      use_enc[1] = use_enc[0] = 0;
      rval.nbytes = 2;
    }
  }
  return rval;
}


/* ************************************************************************* *
 * class TLVLayoutEncoder implementation
 * ************************************************************************* */

//Calculates TLV layout (tag & length octets + EOC for each tag) basing on
//given 'V' part encoding properties.
//Returns  TLVProperty for outermost tag
const TLVStruct & 
  TLVLayoutEncoder::calculateLayout(bool calc_indef/* = false*/)
{
  //calculate value encoding properties first
  _valEnc->calculateVAL(_vProp, _vRule, calc_indef);

  if (!_effTags || _effTags->empty()) {
    _tlws.reserve(1);
    _tlws[0].init(_vProp._ldForm, _vProp._valLen, _vProp._isConstructed);
  } else {
    uint8_t tagIdx = _effTags->size();
    TSLength valLen = _vProp._valLen;
    bool constructedness = _vProp._isConstructed;

    _tlws.reserve(_effTags->size()); //cann't fail here
    //traverse tags from inner to outer
    do
    { //NOTE: keep the same LD form for all LDs in tagging
      _tlws[--tagIdx].init(_vProp._ldForm, valLen, constructedness);
      _tlws[tagIdx].calculate((*_effTags)[tagIdx]);
      valLen = _tlws[tagIdx].getTLVsize();
      constructedness = true;
      _szoBOC += _tlws[tagIdx].getBOCsize();
    } while (tagIdx);
  }
  return _tlws[0];
}

//Encodes 'TL'-part ('begin-of-content' octets)
//NOTE: TLVLayout must be calculated.
ENCResult TLVLayoutEncoder::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (_effTags) { //traverse tags from outer to inner
    for (uint8_t i = 0; (i < _effTags->size()) && rval.isOk(); ++i) {
      rval += _tlws[i].encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    }
  }
  return rval;
}

//Encodes 'end-of-content' octets of encoding (EOC-part)
//NOTE: TLVLayout must be calculated.
ENCResult TLVLayoutEncoder::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (_effTags && !_effTags->empty() && _tlws[0].getEOCsize()) {
    //traverse tags from inner to outer
    uint8_t i = _effTags->size();
    while (--i && rval.isOk()) {
      rval += _tlws[i].encodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    }
  }
  return rval;
}

//Calculates TLV layout octets (length octets + EOC for each tag) basing
//on encoding properties implied by addressed value.
//Returns  TLVProperty for outermost tag.
// 
//NOTE: if 'calc_indef' is set, then full TLV encoding length is calculated even
//if indefinite LD form is used. This may be required if this TLV is enclosed by
//another that uses definite LD form.
const TLVStruct & 
  TLVLayoutEncoder::calculateTLV(bool calc_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_valEnc)
    throw smsc::util::Exception("ber::TLVLayoutEncoder: ValueEncoderIface isn't set");
  return calculateLayout(calc_indef);
}


//Encodes by BER/DER/CER the previously calculated TLV layout (composes complete TLV encoding).
//NOTE: Throws in case the layout wasn't calculated.
ENCResult TLVLayoutEncoder::encodeTLV(uint8_t * use_enc, TSLength max_len) const
  /*throw(std::exception)*/
{
  if (!isCalculated())
    throw smsc::util::Exception("ber::TLVLayoutEncoder: layout isn't calculated");

  ENCResult rval = encodeBOC(use_enc, max_len);
  if (rval.isOk()) {
    rval += _valEnc->encodeVAL(use_enc + rval.nbytes, max_len - rval.nbytes); //throws
    if (rval.isOk())
      rval += encodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}


/* ************************************************************************* *
 * class TypeEncoderAC implementation
 * ************************************************************************* */
const TLVStruct & TypeEncoderAC::prepareTLV(bool calc_indef) /*throw(std::exception)*/
{
  if (!_tlvEnc.getVALEncoder()) //assertion
    throw smsc::util::Exception("ber::TypeEncoderAC: ValueEncoderIface isn't set");

  _tlvEnc.setTagging(TypeTagging::refreshTagging());
  return _tlvEnc.calculateTLV(calc_indef);
}

ENCResult TypeEncoderAC::calculate(void) /*throw(std::exception)*/
{
  ENCResult res(ENCResult::encOk, prepareTLV(true).getTLVsize());
  if (!res.nbytes) //TLV encoding cann't be of zero octets
    res.status = ENCResult::encBadVal;
  return res;
}

ENCResult TypeEncoderAC::encode(uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/
{
  if (!_tlvEnc.isCalculated())  //single pass encoing: caller doesn't care
    prepareTLV(false);          //about resulting TLV encoding length
  return _tlvEnc.encodeTLV(use_enc, max_len);
}

//Compares two type encodings by their ordinal tags
bool TypeEncoderAC::operator<(const TypeEncoderAC & cmp_tenc) const /*throw(std::exception)*/
{
  if (getTSRule() != cmp_tenc.getTSRule())
    throw smsc::util::Exception("ber::TypeEncoderAC::operator<(): TransferSyntax rules are different");

  const ASTag * ownTag = getOrdinalTag();
  if (!ownTag)
    throw smsc::util::Exception("ber::TypeEncoderAC::operator<(): own tag isn't defined");

  const ASTag * cmpTag = cmp_tenc.getOrdinalTag();
  if (!cmpTag)
    throw smsc::util::Exception("ber::TypeEncoderAC::operator<(): tag is to compare isn't defined");

  return *ownTag < *cmpTag;
}


} //ber
} //asn1
} //eyeline

