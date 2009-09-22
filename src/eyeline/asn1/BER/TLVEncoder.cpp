#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/TLVEncoder.hpp"
#include "eyeline/asn1/BER/EncodeSubId_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag:
 * Returns  number of bytes of resulted encoding or zero.
 * ************************************************************************* */
uint16_t estimate_tag(const ASTag & use_tag)
{
  if (use_tag._tagValue <= 30)  //short tag form
    return 1;
  return estimate_identifier(use_tag._tagValue);
}

/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.680 clause 8.1.2.
 * ************************************************************************* */
ENCResult encode_tag(const ASTag & use_tag, uint8_t * use_enc, TSLength max_len)
{
  ENCResult rval(ENCResult::encOk);

  if (!max_len || !use_enc) {
    rval.status = ENCResult::encBadArg;
  } else if (use_tag._tagValue <= 30) {
    //short tag form
    *use_enc = ((uint8_t)use_tag._tagClass << 6) | (uint8_t)use_tag._tagValue;
    if (use_tag._isConstructed)
      *use_enc |= 0x20;
    rval.nbytes = 1;
  } else {
    //long tag form (tag > 30)
    if (!(rval.nbytes = encode_identifier(use_tag._tagValue, use_enc + 1, max_len - 1))) {
      rval.status = ENCResult::encMoreMem;
    } else {
      ++rval.nbytes;
      //compose 1st byte
      *use_enc = ((uint8_t)use_tag._tagClass << 6) | 0x1F;
      if (use_tag._isConstructed)
        *use_enc |= 0x20;
    }
  }
  return rval;
}


/* ************************************************************************* *
 * Class LDeterminant implementation:
 * ************************************************************************* */
void LDeterminant::composeBOC(void)
{
  if (_defForm)
    _bocNum = (uint8_t)encode_identifier(_vlen, _boc, sizeof(_boc));
  else {
    _boc[0] = 0x80;
    _bocNum = 1;
  }
}

//Returns number of 'begin-of-content' octets
ENCResult LDeterminant::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);

  if (max_len < _bocNum) {
    rval.status = ENCResult::encMoreMem;
  } else {
    for (uint8_t i = 0; i < _bocNum; ++i)
      use_enc[i] = _boc[i];
    rval.nbytes = _bocNum;
  }
  return rval;
}

//Encodes 'end-of-content' octets of length determinant encoding
ENCResult LDeterminant::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);

  if (!_defForm) {
    if (max_len < 2) {
      rval.status = ENCResult::encMoreMem;
    } else {
      use_enc[1] = *use_enc = 0;
      rval.nbytes = 2;
    }
  }
  return rval;
}


/* ************************************************************************* *
 * class ASTaggingLayout implementation
 * ************************************************************************* */
//Calculates TLV layout (length of 'TL' pair for each tag) basing on
//given length of 'V' part encoding and length determinants form setting.
//Returns  number of bytes of resulted 'TLV' encoding.
TSLength TLVLayout::calculateFor(TSLength value_len)
{
  _octsTLV = 0;
  uint8_t tagIdx = size();

  while (tagIdx)
  {
    _dets[tagIdx] = value_len;
    uint16_t tlLen = _dets[tagIdx].octetsLD();
    tlLen += estimate_tag(_tags[--tagIdx]);
    
    value_len += tlLen;
    _octsTLV += tlLen;
  }
  return _octsTLV;
}

//Encodes 'begin-of-content' octets ('TL'-part of encoding)
ENCResult TLVLayout::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);

  if (!_octsTLV || !size()) {
    rval.status = ENCResult::encBadArg;
    return rval;
  }
  if (octetsBOC() > max_len) {
    rval.status = ENCResult::encMoreMem;
    return rval;
  }

  for (uint8_t tagIdx = 0; tagIdx < size(); ++tagIdx) {
    TSLength  encLen = rval.nbytes;
    rval = encode_tag(_tags[tagIdx], use_enc + encLen, max_len - encLen);
    rval.nbytes += encLen;
    if (rval.status != ENCResult::encOk)
      break;

    encLen = rval.nbytes;
    rval = _dets[tagIdx].encodeBOC(use_enc + encLen, max_len - encLen);
    rval.nbytes += encLen;
  }
  return rval;
}

//Encodes 'end-of-content' octets of encoding (EOC-part)
ENCResult TLVLayout::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);

  if (!_octsTLV || !size()) {
    rval.status = ENCResult::encBadArg;
    return rval;
  }
  if (octetsEOC() > max_len) {
    rval.status = ENCResult::encMoreMem;
    return rval;
  }

  uint8_t tagIdx = size();
  do {
    TSLength  encLen = rval.nbytes;
    rval = _dets[--tagIdx].encodeEOC(use_enc + encLen, max_len - encLen);
    rval.nbytes += encLen;
  } while (tagIdx && (rval.status == ENCResult::encOk));

  return rval;
}

} //ber
} //asn1
} //eyeline

