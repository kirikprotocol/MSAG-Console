#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class BITFragment : public TLEncoder
{
  public:
    BITFragment(TSLength val_len = 1000) : TLEncoder()
    {
      init(LDeterminant::frmDefinite, val_len, false);
      compose(_tagBITSTR);
    }
};

static BITFragment _bitFragm;

/* ************************************************************************* *
 * Class EncoderOfOCTSTR implementation:
 * ************************************************************************* */
TSLength EncoderOfBITSTR::calculateFragments(void) const
  /*throw(std::exception)*/
{
  BITFragment lastFrag(_encValSz%999 + 1);
  TSLength rval = lastFrag.getTLVsize() + _bitFragm.getTLVsize()*numFragments();
  if (rval < _encValSz) //check for TSLength overloading
    throw smsc::util::Exception("EncoderOfBITSTR: value is too large");
  return rval;
}

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --

const EncodingProperty &
  EncoderOfBITSTR::calculateVAL(bool calc_indef/* = false*/)
    /*throw(std::exception)*/
{
  if (getRule() == TSGroupBER::ruleCER) { //perform fragmentation if necessary
    if (numFragments() > 1) {
      _vProp._isConstructed = true;
      _vProp._ldForm = LDeterminant::frmIndefinite;
      _vProp._valLen = calculateFragments();
      _isCalculated = true;
      return _vProp;
    }
  }
  //default form
  _vProp._isConstructed = false;
  _vProp._ldForm = LDeterminant::frmDefinite;
  _vProp._valLen = (TSLength)_encValSz + 1;
  _isCalculated = true;
  return _vProp;
}

ENCResult
  EncoderOfBITSTR::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);

  if (_vProp._isConstructed) { //fragmented encoding
    BITSTR::size_type   i = 0;
    BITSTR::size_type   maxIdx = 999*numFragments();

    while (i < maxIdx) {
      rval += _bitFragm.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
      if (!rval.isOk())
        return rval;
      if ((max_len - rval.nbytes) < 1000)  {
        rval.status = ENCResult::encMoreMem;
        return rval;
      }
      use_enc[0] = 0; //no unused bits in last octet of intermediate fragments
      ++(rval.nbytes);
      memcpy(use_enc + rval.nbytes, _encVal + i, 999);
      rval.nbytes += 999; i += 999;
    }
    //encode last fragment
    BITFragment lastFrag(_encValSz%999 + 1);
    if ((max_len - rval.nbytes) < lastFrag.getTLVsize())  {
      rval.status = ENCResult::encMoreMem;
      return rval;
    }
    rval += lastFrag.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    if (!rval.isOk())
      return rval;

    use_enc[0] = unusedBits(); //unused bits in last octet of last fragment
    ++(rval.nbytes);
    memcpy(use_enc + rval.nbytes, _encVal + i, lastFrag._valLen - 1);
    rval.nbytes += lastFrag._valLen - 1;
  } else {
    if (max_len < (_encValSz + 1))
      rval.status = ENCResult::encMoreMem;
    else {
      use_enc[0] = unusedBits(); //unused bits in last octet of last fragment
      ++(rval.nbytes);
      memcpy(use_enc + rval.nbytes, _encVal, _encValSz);
      rval.nbytes += _encValSz;
    }
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

