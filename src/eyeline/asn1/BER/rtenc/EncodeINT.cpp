#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeInt_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfINTEGER implementation:
 * ************************************************************************* */
//NOTE: encoding of INTEGER type value has the same form for all BER
//family rules: primitive encoding with definite LD form

// -- -------------------------------------- --
// -- ValueEncoderIface interface methods
// -- -------------------------------------- --

void EncoderOfINTEGER::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                    bool do_indef/* = false*/) /*throw(std::exception)*/
{
  switch (_vSzo) {
  case szo32: {
    val_prop._valLen = _signed ? estimate_signed_INTEGER(_encVal.i32)
                                : estimate_unsigned_INTEGER(_encVal.u32);
  } break;

  case szo16: {
    val_prop._valLen = _signed ? estimate_signed_INTEGER(_encVal.i16)
                                : estimate_unsigned_INTEGER(_encVal.u16);
  } break;
 
  case szo8: {
    val_prop._valLen = _signed ? estimate_signed_INTEGER(_encVal.i8)
                                : estimate_unsigned_INTEGER(_encVal.u8);
  } break;

  default: //szoNone
    throw smsc::util::Exception("asn1::ber::EncoderOfINTEGER: value isn't set");
  }
  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._isConstructed = false;
}

ENCResult 
  EncoderOfINTEGER::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);

  switch (_vSzo) {
  case szo32: {
    rval.nbytes = _signed ? encode_signed_INTEGER(_encVal.i32, use_enc, max_len)
                          : encode_unsigned_INTEGER(_encVal.u32, use_enc, max_len);
  } break;

  case szo16: {
    rval.nbytes = _signed ? encode_signed_INTEGER(_encVal.i16, use_enc, max_len)
                          : encode_unsigned_INTEGER(_encVal.u16, use_enc, max_len);
  } break;

  case szo8: {
    rval.nbytes = _signed ? encode_signed_INTEGER(_encVal.i8, use_enc, max_len)
                          : encode_unsigned_INTEGER(_encVal.u8, use_enc, max_len);
  } break;

  default: //szoNone
    rval.status = ENCResult::encBadVal;
  }
  if (!rval.nbytes)
    rval.status = ENCResult::encMoreMem;
  return rval;
}

} //ber
} //asn1
} //eyeline

