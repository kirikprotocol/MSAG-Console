#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/util/IntConv.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeIdents_impl.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeLDs_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag:
 * Returns  number of bytes of resulted encoding or zero.
 * ************************************************************************* */
uint8_t estimate_tag(const ASTag & use_tag)
{
  if (use_tag._tagValue <= 30)  //short tag form
    return 1;
  return estimate_identifier(use_tag._tagValue);
}

/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
 * Returns zero in case of insuffient buffer is provided.
 * NOTE: 'max_len' argument must be grater then zero
 * ************************************************************************* */
static uint8_t encode_tag_internal(const ASTag & use_tag, bool is_constructed,
                                   uint8_t * use_enc, uint8_t max_len)
{
  uint8_t rval = 0;
  if (use_tag._tagValue <= 30) {
    //short tag form, compose 1st byte
    *use_enc = (uint8_t)use_tag._tagValue;
  } else {
    //long tag form (tag > 30)
    if (!(rval = encode_identifier(use_tag._tagValue, use_enc + 1, max_len - 1)))
      return 0;
    //compose 1st byte
    *use_enc = 0x1F;
  }
  //complete 1st byte: class & constructedness
  *use_enc |= (uint8_t)use_tag._tagClass << 6;
  if (is_constructed)
    *use_enc |= 0x20;
  ++rval;
  return rval;
}

/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
 * ************************************************************************* */
ENCResult encode_tag(const ASTag & use_tag, bool is_constructed, 
                     uint8_t * use_enc, TSLength max_len)
{
  ENCResult rval(ENCResult::encOk);

  if (!max_len || !use_enc) {
    rval.status = ENCResult::encBadArg;
  } else if (!(rval.nbytes = encode_tag_internal(use_tag, is_constructed, use_enc,
                                                 DOWNCAST_UNSIGNED(max_len, uint8_t)))) {
    rval.status = ENCResult::encMoreMem;
  }
  return rval;
}

/* ************************************************************************* *
 * Class TLVProperty implementation:
 * ************************************************************************* */

void TLVProperty::calculate(const ASTag & use_tag)
{
  _szoTag = estimate_tag(use_tag);
  _szoLOC = isDefinite() ? estimate_ldeterminant(_valLen) : 1;
}


/* ************************************************************************* *
 * Class TLEncoder implementation:
 * ************************************************************************* */
void TLEncoder::compose(const ASTag & use_tag)
{
  //here encode_tag_internal() cannt' fail, otherwise assertion!
  _szoTag = encode_tag_internal(use_tag, _isConstructed, _octTag, (uint8_t)sizeof(_octTag));
  if (isDefinite()) {
    //here encode_identifier() cannt' fail, otherwise assertion!
    _szoLOC = encode_ldeterminant(_valLen, _octLOC, (uint8_t)sizeof(_octLOC));
  } else {
    _octLOC[0] = 0x80;
    _szoLOC = 1;
  }
}

//Encodes 'begin-of-content' octets of TLV encoding
ENCResult TLEncoder::encodeBOC(uint8_t * use_enc, TSLength max_len) const
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
  }
  return rval;
}

//Encodes 'end-of-content' octets of TLV encoding
ENCResult TLEncoder::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (max_len < getEOCsize()) {
    rval.status = ENCResult::encMoreMem;
  } else if (getEOCsize()) { //indefinite form of LD
    use_enc[1] = use_enc[0] = 0;
    rval.nbytes = 2;
  }
  return rval;
}


/* ************************************************************************* *
 * class TLVLayoutEncoder implementation
 * ************************************************************************* */
void TLVLayoutEncoder::initTypeLayout(const ASTagging & use_tags, ValueEncoderAC & use_val_enc)
{
  _effTags = use_tags;
  _tlws.enlarge(_effTags.size());
  _szoBOC = 0;
  _valEnc = &use_val_enc;
}

void TLVLayoutEncoder::initFieldLayout(const TLVLayoutEncoder & type_enc,
                                       const ASTagging * fld_tags/* = NULL*/)
{
  if (fld_tags)
    _effTags = *fld_tags;
  _effTags.conjoin(type_enc.getTagging());
  _tlws.enlarge(_effTags.size());
  _szoBOC = 0;
  _valEnc = type_enc.getValueEncoder();
}

//Calculates TLV layout (tag & length octets + EOC for each tag) basing on
//given 'V' part encoding properties.
//Returns  EncodingProperty for outermost tag
const TLVProperty & 
  TLVLayoutEncoder::calculateLayout(const EncodingProperty & val_prop)
{
  if (!_effTags.size())
    _tlws[0].reset();
  else {
    uint8_t tagIdx = _effTags.size();
    TSLength valLen = val_prop._valLen;
    bool constructedness = val_prop._isConstructed;
  
    //traverse tags from inner to outer
    while (--tagIdx)
    { //NOTE: keep the same LD form for all LDs in tagging
      _tlws[tagIdx].init(val_prop._ldForm, valLen, constructedness);
      _tlws[tagIdx].calculate(_effTags[tagIdx]);
      valLen = _tlws[tagIdx].getTLVsize();
      constructedness = true;
      _szoBOC += _tlws[tagIdx].getBOCsize();
    }
  }
  return _tlws[0];
}

//Encodes 'TL'-part ('begin-of-content' octets)
//NOTE: TLVLayout must be calculated.
ENCResult TLVLayoutEncoder::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  //traverse tags from outer to inner
  for (uint8_t i = 0; (i < _effTags.size()) && rval.isOk(); ++i) {
    rval += _tlws[i].encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}

//Encodes 'end-of-content' octets of encoding (EOC-part)
//NOTE: TLVLayout must be calculated.
ENCResult TLVLayoutEncoder::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (_effTags.size() && _tlws[0].getEOCsize()) {
    //traverse tags from inner to outer
    uint8_t i = _effTags.size();
    while (--i && rval.isOk()) {
      rval += _tlws[i].encodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    }
  }
  return rval;
}

//Encodes by BER/DER/CER the previously calculated TLV layout (composes complete TLV encoding).
//NOTE: Throws in case the layout wasn't calculated.
ENCResult TLVLayoutEncoder::encodeCalculated(uint8_t * use_enc, TSLength max_len) const
  /*throw(std::exception)*/
{
  if (!_valEnc->isCalculated()) //caller doesn't care about TLV encoding length
    throw smsc::util::Exception("TLVLayoutEncoder: layout isn't calculated");

  ENCResult rval = encodeBOC(use_enc, max_len);
  if (rval.isOk()) {
    rval += _valEnc->encodeVAL(use_enc + rval.nbytes, max_len - rval.nbytes); //throws
    if (rval.isOk())
      rval += encodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}

//Encodes by BER/DER/CER the TLV layout (composes complete TLV encoding).
//NOTE: Throws in case of value that cann't be encoded.
//NOTE: Calculates layout if it wasn't calculated yet
ENCResult TLVLayoutEncoder::encodeTLV(uint8_t * use_enc, TSLength max_len)
  /*throw(std::exception)*/
{
  if (!_valEnc->isCalculated()) //caller doesn't care about TLV encoding length
    calculateTLV();
  return encodeCalculated(use_enc, max_len);
}

} //ber
} //asn1
} //eyeline

