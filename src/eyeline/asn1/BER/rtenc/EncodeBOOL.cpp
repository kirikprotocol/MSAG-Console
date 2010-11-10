#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeBOOL.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfBOOL implementation:
 * ************************************************************************* */

// -- -------------------------------------- --
// -- ValueEncoderIface interface methods
// -- -------------------------------------- --
void EncoderOfBOOL::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                 bool calc_indef/* = false*/)
  /*throw(std::exception)*/
{
  val_prop._isConstructed = false;
  val_prop._valLen = 1;
  val_prop._ldForm = LDeterminant::frmDefinite;
}

ENCResult EncoderOfBOOL::encodeVAL(uint8_t * use_enc, TSLength max_len) const
  /*throw(std::exception)*/
{
  *use_enc = !_encVal ? 0x00 : 0xFF;
  return ENCResult(ENCResult::encOk, 1);
}

} //ber
} //asn1
} //eyeline

