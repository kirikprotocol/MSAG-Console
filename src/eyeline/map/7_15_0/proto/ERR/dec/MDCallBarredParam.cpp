#include "MDCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDCallBarredParam::construct()
{
  setAlternative(0, asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  setAlternative(1, asn1::_tagSEQOF, asn1::ASTagging::tagsIMPLICIT);
}

asn1::ber::TypeDecoderAC *
MDCallBarredParam::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDCallBarredParam::prepareAlternative: value isn't set!");
  if (unique_idx > 1) //assertion!!!
    throw smsc::util::Exception("MDCallBarredParam::prepareAlternative: undefined UId");

  cleanup();

  if (unique_idx == 0)
    _pDec._callBarringCause= new (_memAlt._buf) MDCallBarringCause();
  else
    _pDec._extCallBarredParam= new (_memAlt._buf) MDExtensibleCallBarredParam();

  return _pDec._any;
}

}}}}
