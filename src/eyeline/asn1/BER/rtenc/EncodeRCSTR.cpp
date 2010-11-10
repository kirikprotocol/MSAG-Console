#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class OCTFragment : public TLComposer {
public:
  explicit OCTFragment(TSLength val_len = 1000) : TLComposer()
  {
    init(LDeterminant::frmDefinite, val_len, false);
    calculate(_tagOCTSTR);
  }
};

static OCTFragment _octFragm;

/* ************************************************************************* *
 * Class EncoderOfRCSTR implementation:
 * ************************************************************************* */
TSLength EncoderOfRCSTR::calculateFragments(void) const
  /*throw(std::exception)*/
{
  OCTFragment lastFrag(_encValSz%1000);
  TSLength rval = lastFrag.getTLVsize() + _octFragm.getTLVsize()*numFragments();
  if (rval < _encValSz) //check for TSLength overloading
    throw smsc::util::Exception("EncoderOfRCSTR: value is too large");
  return rval;
}

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --

void
  EncoderOfRCSTR::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                               bool calc_indef/* = false*/) /*throw(std::exception)*/
{
  _encValSz = _strConv->getPackedLength();
  if (use_rule == TSGroupBER::ruleCER) { //perform fragmentation if necessary
    if (numFragments() > 1) {
      val_prop._isConstructed = true;
      val_prop._ldForm = LDeterminant::frmIndefinite;
      val_prop._valLen = calculateFragments();
      return;
    }
  }
  //default form
  val_prop._isConstructed = false;
  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._valLen = (TSLength)_encValSz;
}

ENCResult
  EncoderOfRCSTR::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  //assert(_isCalculated);
  ENCResult rval(ENCResult::encOk);

  if (_tlvEnc.getVALProperty()._isConstructed) { //fragmented encoding
    for (TSLength i = 1; i < numFragments(); ++i) {
      rval += _octFragm.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
      if (!rval.isOk())
        return rval;
      if ((max_len - rval.nbytes) < 1000)  {
        rval.status = ENCResult::encMoreMem;
        return rval;
      }
      rval.nbytes += _strConv->pack2Octs(use_enc + rval.nbytes, 1000);
    }
    //encode last fragment
    OCTFragment lastFrag(_encValSz%1000);
    if ((max_len - rval.nbytes) < lastFrag.getTLVsize())  {
      rval.status = ENCResult::encMoreMem;
      return rval;
    }
    rval += lastFrag.encodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    if (!rval.isOk())
      return rval;
    rval.nbytes += _strConv->pack2Octs(use_enc + rval.nbytes, lastFrag._valLen);

  } else {
    if (max_len < _encValSz)
      rval.status = ENCResult::encMoreMem;
    else
      rval.nbytes += _strConv->pack2Octs(use_enc, _encValSz);
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

