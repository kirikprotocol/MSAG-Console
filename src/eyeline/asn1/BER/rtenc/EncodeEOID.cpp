#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfEOID implementation:
 * ************************************************************************* */

// -- -------------------------------------- -- 
// -- ValueEncoderIface interface methods       
// -- -------------------------------------- -- 

void EncoderOfEOID::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                 bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_encVal)
    throw smsc::util::Exception("ber::EncoderOfEOID: value isn't set");
  //NOTE: encoding of ObjectID type value has the same form for all BER
  //family rules: primitive encoding with definite LD form
  val_prop._valLen = _encVal->length();
  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._isConstructed = false;
}

ENCResult
  EncoderOfEOID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);
  if (max_len < _encVal->length()) {
    rval.status = ENCResult::encMoreMem;
  } else {
    memcpy(use_enc, _encVal->octets(), rval.nbytes = _encVal->length());
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

