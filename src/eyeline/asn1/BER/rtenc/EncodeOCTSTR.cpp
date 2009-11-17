#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class OCTFragment : public TLEncoder
{
  public:
    OCTFragment(TSLength val_len = 1000) : TLEncoder()
    {
      init(LDeterminant::frmDefinite, val_len, false);
      compose(_tagOCTSTR);
    }
};

static OCTFragment _octFragm;

/* ************************************************************************* *
 * Class EncoderOfOCTSTR implementation:
 * ************************************************************************* */
TSLength EncoderOfOCTSTR::calculateFragments(void) const
  /*throw(std::exception)*/
{
  OCTFragment lastFrag(_encValSz%1000);
  TSLength rval = lastFrag.getTLVsize() + _octFragm.getTLVsize()*numFragments();
  if (rval < _encValSz) //check for TSLength overloading
    throw smsc::util::Exception("EncoderOfOCTSTR: value is too large");
  return rval;
}

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --

const EncodingProperty &
  EncoderOfOCTSTR::calculateVAL(bool calc_indef/* = false*/) const
    /*throw(std::exception)*/
{
  if (getRule() == TSGroupBER::ruleCER) { //perform fragmentation if necessary
    if (numFragments() > 1) {
      _vProp._isConstructed = true;
      _vProp._ldForm = LDeterminant::frmIndefinite;
      _vProp._valLen = calculateFragments();
    }
  }
  //default form
  _vProp._isConstructed = false;
  _vProp._ldForm = LDeterminant::frmDefinite;
  _vProp._valLen = (TSLength)_encValSz;

  _isCalculated = true;
  return _vProp;
}

ENCResult
  EncoderOfOCTSTR::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);
  if (!_isCalculated)
    calculateVAL();

  if (_vProp._isConstructed) { //fragmented encoding
    OCTSTR::size_type   i = 0;
    OCTSTR::size_type   maxIdx = 1000*numFragments();

    while (i < maxIdx) {
      rval += _octFragm.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
      if (!rval.isOk())
        return rval;
      if ((max_len - rval.nbytes) < 1000)  {
        rval.status = ENCResult::encMoreMem;
        return rval;
      }
      memcpy(use_enc + rval.nbytes, _encVal + i, 1000);
      rval.nbytes += 1000; i += 1000;
    }
    //encode last fragment
    OCTFragment lastFrag(_encValSz%1000);
    if ((max_len - rval.nbytes) < lastFrag._valLen)  {
      rval.status = ENCResult::encMoreMem;
      return rval;
    }
    rval += lastFrag.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    if (!rval.isOk())
      return rval;
    memcpy(use_enc + rval.nbytes, _encVal + i, lastFrag._valLen);
    rval.nbytes += lastFrag._valLen;
  } else {
    if (max_len < _encValSz)
      rval.status = ENCResult::encMoreMem;
    else
      memcpy(use_enc, _encVal, rval.nbytes = _encValSz);
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

