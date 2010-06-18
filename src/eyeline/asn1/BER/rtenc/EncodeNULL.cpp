#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfNULL implementation:
 * ************************************************************************* */
// -- -------------------------------------- --
// -- ValueEncoderIface interface methods
// -- -------------------------------------- --
void
  EncoderOfNULL::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                              bool calc_indef/* = false*/) /*throw(std::exception)*/
{
  val_prop._isConstructed = false;
  val_prop._valLen = 0;
  val_prop._ldForm = LDeterminant::frmDefinite;
}

} //ber
} //asn1
} //eyeline

