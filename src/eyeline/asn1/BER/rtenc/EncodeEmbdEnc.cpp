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
  _altEnc._astype = new (_memAlt._buf) EncoderOfASType(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule());
  _kind = asn1::EmbeddedEncoding::evASType;
  _altEnc._astype->setValue(use_val);
  setSelection(*_altEnc._astype);
}

void EncoderOfEmbdEncoding::setValue(const asn1::TransferSyntax & use_val)
  /*throw(std::exception)*/
{
  _altEnc._astype = new (_memAlt._buf) EncoderOfASType(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule());
  _kind = asn1::EmbeddedEncoding::evASType;
  _altEnc._astype->setValue(use_val);
  setSelection(*_altEnc._astype);
}

void EncoderOfEmbdEncoding::setValue(const asn1::OCTSTR::ArrayType & use_val)
  /*throw(std::exception)*/
{
  _altEnc._octstr = new (_memAlt._buf) EncoderOfOCTSTR(_tagOctetAligned, ASTagging::tagsIMPLICIT, getTSRule());
  _kind = asn1::EmbeddedEncoding::evOctAligned;
  _altEnc._octstr->setValue(use_val);
  setSelection(*_altEnc._octstr);
}

void EncoderOfEmbdEncoding::setValue(const asn1::BITSTR::ArrayType & use_val)
  /*throw(std::exception)*/
{
  _altEnc._bitstr = new (_memAlt._buf) EncoderOfBITSTR(_tagBitAligned, ASTagging::tagsIMPLICIT, getTSRule());
  _kind = asn1::EmbeddedEncoding::evBitAligned;
  _altEnc._bitstr->setValue(use_val);
  setSelection(*_altEnc._bitstr);
}

void EncoderOfEmbdEncoding::setValue(const asn1::EmbeddedEncoding & use_val)
  /*throw(std::exception)*/
{
  switch (use_val.getKind()) {
  case asn1::EmbeddedEncoding::evBitAligned: {
    setValue(*use_val.getBITS());
    return;
  }
  case asn1::EmbeddedEncoding::evOctAligned: {
    setValue(*use_val.getOCTS());
    return;
  }
  default: //asn1::EmbeddedEncoding::evASType:
    if (use_val.getTS())
      setValue(*use_val.getTS());
  } /*eosw*/
}


} //ber
} //asn1
} //eyeline

