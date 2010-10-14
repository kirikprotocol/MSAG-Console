#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeROID.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSubIds.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfRelativeOID implementation:
 * ************************************************************************* */

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
//NOTE: encoding of RelativeOID type value has the same form for all BER
//family rules: primitive encoding with definite LD form

void
  EncoderOfRelativeOID::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                     bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_encVal)
    throw smsc::util::Exception("ber::EncoderOfROID: value isn't set");

  if (!(val_prop._valLen = estimate_SubIds(_encVal->get(), _encVal->size())))
    throw smsc::util::Exception("EncoderOfRelativeOID: illegal value");

  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._isConstructed = false;
}

ENCResult
  EncoderOfRelativeOID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  return encode_SubIds(_encVal->get(), _encVal->size(), use_enc, max_len);
}

} //ber
} //asn1
} //eyeline

