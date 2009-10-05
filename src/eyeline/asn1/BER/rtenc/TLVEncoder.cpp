#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeIdents_impl.hpp"

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
 * Class LDEncoder implementation:
 * ************************************************************************* */
uint8_t LDEncoder::composeBOC(void)
{
  if (_defForm) {
    //here encode_identifier() cannt' fail
    _bocSZO = encode_identifier(_vlen, _boc, sizeof(_boc));
  } else {
    _boc[0] = 0x80;
    _bocSZO = 1;
  }
  return _bocSZO;
}

//Returns number of 'begin-of-content' octets
ENCResult LDEncoder::encodeBOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);

  uint8_t ldLen = _bocSZO;
  if (!ldLen) { //BOC wasn't composed earlier
    if (!(ldLen = encode_identifier(_vlen, use_enc, max_len)))
      rval.status = ENCResult::encMoreMem;
    else
      rval.nbytes = ldLen;
  } else {      //using precomposed BOC octets
    if (max_len < _bocSZO) {
      rval.status = ENCResult::encMoreMem;
    } else {
      for (uint8_t i = 0; i < _bocSZO; ++i)
        use_enc[i] = _boc[i];
      rval.nbytes = _bocSZO;
    }
  }
  return rval;
}

//Encodes 'end-of-content' octets of length determinant encoding
ENCResult LDEncoder::encodeEOC(uint8_t * use_enc, TSLength max_len) const
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
 * Class TLPairEncoder implementation:
 * ************************************************************************* */
//Returns number of 'TL'-part octets ('begin-of-content')
uint8_t TLPairEncoder::getTLsize(void)
{
  return _tldSZO ? _tldSZO : (_tldSZO = (composeBOC() + estimate_tag(*this)));
}

ENCResult TLPairEncoder::encodeTL(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval = encode_tag(*this, use_enc, max_len);
  if (rval.isOk())
    rval += encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  return rval;
}

/* ************************************************************************* *
 * class TLVLayoutEncoder implementation
 * ************************************************************************* */
//Calculates TLV layout (length of 'TL' pair for each tag) basing on
//given length of 'V' part encoding and length determinants form setting.
//Returns  number of bytes of resulted 'TLV' encoding.
TSLength TLVLayoutEncoder::calculate(void)
{
  _tldSZO = 0;
  TSLength valueLen = getValueLen(); //'V'-part encoding length for current tag
  uint8_t tagIdx = size();

  while (tagIdx)
  {
    uint8_t tlLen = at(--tagIdx).getTLsize();
    valueLen += tlLen;
    _tldSZO += tlLen;
    if (tagIdx)
      at(tagIdx-1).setValueLen(valueLen);
  }
  return _tldSZO;
}


//Returns number of 'end-of-content' octets (EOC-part)
TSLength TLVLayoutEncoder::getEOCsize(void) const
{
  TSLength rval = 0;
  for (uint8_t i = 0; i < size(); ++i)
    rval += (this->operator[](i)).getEOCsize();
  return rval;
}


//Encodes 'TL'-part ('begin-of-content' octets)
//Perfoms calculation of TLVLayout id necessary
ENCResult TLVLayoutEncoder::encodeTL(uint8_t * use_enc, TSLength max_len)
{
  ENCResult rval(ENCResult::encOk);
  if (getTLsize() > max_len) {
    rval.status = ENCResult::encMoreMem;
  } else {
    for (uint8_t tagIdx = 0; (tagIdx < size()) && rval.isOk(); ++tagIdx) {
      rval += get()[tagIdx].encodeTL(use_enc + rval.nbytes, max_len - rval.nbytes);
    }
  }
  return rval;
}

//Encodes 'end-of-content' octets of encoding (EOC-part)
ENCResult TLVLayoutEncoder::encodeEOC(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (!_tldSZO) {
    rval.status = ENCResult::encBadArg;
    return rval;
  }
  if (_tldSZO > max_len) {
    rval.status = ENCResult::encMoreMem;
    return rval;
  }

  uint8_t tagIdx = size();
  do {
    rval += at(--tagIdx).encodeTL(use_enc + rval.nbytes, max_len - rval.nbytes);
  } while (tagIdx && rval.isOk());

  return rval;
}

} //ber
} //asn1
} //eyeline

