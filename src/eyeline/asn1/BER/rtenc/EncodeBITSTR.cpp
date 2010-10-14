#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class BITFragment : public TLComposer
{
  public:
    BITFragment(TSLength val_len = 1000) : TLComposer()
    {
      init(LDeterminant::frmDefinite, val_len, false);
      compose(_tagBITSTR);
    }
};

static BITFragment _bitFragm;

/* ************************************************************************* *
 * Class EncoderOfBITSTR implementation:
 * ************************************************************************* */
TSLength EncoderOfBITSTR::calculateFragments(void) const
  /*throw(std::exception)*/
{
  BITFragment lastFrag(_encValOcts%999 + 1);
  TSLength rval = lastFrag.getTLVsize() + _bitFragm.getTLVsize()*numFragments();
  if (rval < _encValOcts) //check for TSLength overloading
    throw smsc::util::Exception("EncoderOfBITSTR: value is too large");
  return rval;
}

// -- -------------------------------------- --
// -- ValueEncoderIface interface methods
// -- -------------------------------------- --
void EncoderOfBITSTR::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                bool calc_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_encVal)
    throw smsc::util::Exception("ber::EncoderOfBITSTR: value isn't set");

  if (use_rule == TSGroupBER::ruleCER) { //perform fragmentation if necessary
    if (numFragments() > 1) {
      val_prop._isConstructed = true;
      val_prop._ldForm = LDeterminant::frmIndefinite;
      val_prop._valLen = calculateFragments();
    }
  }
  //default form
  val_prop._isConstructed = false;
  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._valLen = (TSLength)_encValOcts + 1;
}

ENCResult EncoderOfBITSTR::encodeVAL(uint8_t * use_enc, TSLength max_len) const
  /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);

  if (_tlvEnc.getVALProperty()._isConstructed) { //fragmented encoding
    TSLength   i = 0;
    TSLength   maxIdx = 999*numFragments();

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
    BITFragment lastFrag(_encValOcts%999 + 1);
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
    if (max_len < (_encValOcts + 1))
      rval.status = ENCResult::encMoreMem;
    else {
      use_enc[0] = unusedBits(); //unused bits in last octet of last fragment
      ++(rval.nbytes);
      memcpy(use_enc + rval.nbytes, _encVal, _encValOcts);
      rval.nbytes += _encValOcts;
    }
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

