#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfChoice implementation:
 * ************************************************************************* */
void EncoderOfChoice::setAlternative(const ASTagging & use_tags, ValueEncoderIface * val_enc)
{
  if (!val_enc)
    throw smsc::util::Exception("ber::EncoderOfChoice::setSelection(): ValueEncoder isn't defined");

  TypeEncoderAC::init(*val_enc);
  _altTags.setTagging(use_tags);
  refreshTagging(); //compose complete tagging of that CHOICE
}

void EncoderOfChoice::setSelection(TypeEncoderAC & type_enc)
{
  if (!type_enc.getTagging())
    throw smsc::util::Exception("ber::EncoderOfChoice::setSelection(): tagging isn't defined");
  setAlternative(*type_enc.getTagging(), type_enc.getVALEncoder());
}

void EncoderOfChoice::setSelection(TypeEncoderAC & type_enc,
                                   const ASTag & fld_tag,
                                   ASTagging::Environment_e fld_env)
{
  if (!type_enc.getTagging())
    throw smsc::util::Exception("ber::EncoderOfChoice::setSelection(): tagging isn't defined");
  setAlternative(ASTagging(fld_tag, fld_env, *type_enc.getTagging()),
               type_enc.getVALEncoder());
}


} //ber
} //asn1
} //eyeline

