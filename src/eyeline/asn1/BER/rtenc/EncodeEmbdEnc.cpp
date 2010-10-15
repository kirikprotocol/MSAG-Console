#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeEmbdEnc.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfEmbdEncoding implementation:
 * ************************************************************************* */
const ASTag EncoderOfEmbdEncoding::_tagSingleASN1Type(ASTag::tagContextSpecific, 0);
const ASTag EncoderOfEmbdEncoding::_tagOctetAligned(ASTag::tagContextSpecific, 1);
const ASTag EncoderOfEmbdEncoding::_tagBitAligned(ASTag::tagContextSpecific, 2);


void EncoderOfEmbdEncoding::setValue(TypeEncoderAC & use_val)
  /*throw(std::exception)*/
{
  _altEnc.astype().init(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule()).setValue(use_val);
  setSelection(*_altEnc.astype().get());
}

void EncoderOfEmbdEncoding::setValue(const asn1::TransferSyntax & use_val)
  /*throw(std::exception)*/
{
  _altEnc.astype().init(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule()).setValue(use_val);
  setSelection(*_altEnc.astype().get());
}

void EncoderOfEmbdEncoding::setValue(const asn1::OCTSTR::ArrayType & use_val)
  /*throw(std::exception)*/
{
  _altEnc.octstr().init(_tagOctetAligned, ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  setSelection(*_altEnc.octstr().get());
}

void EncoderOfEmbdEncoding::setValue(const asn1::BITSTR::ArrayType & use_val)
  /*throw(std::exception)*/
{
  _altEnc.bitstr().init(_tagBitAligned, ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val);
  setSelection(*_altEnc.bitstr().get());
}

void EncoderOfEmbdEncoding::setValue(const asn1::EmbeddedEncoding & use_val)
  /*throw(std::exception)*/
{
  switch (use_val.getKind()) {
  case asn1::EmbeddedEncoding::evBitAligned: {
    setValue(*use_val.BITS().get());
    return;
  }
  case asn1::EmbeddedEncoding::evOctAligned: {
    setValue(*use_val.OCTS().get());
    return;
  }
  default: //asn1::EmbeddedEncoding::evASType:
    if (use_val.TS().get())
      setValue(*use_val.TS().get());
  } /*eosw*/
}


} //ber
} //asn1
} //eyeline

