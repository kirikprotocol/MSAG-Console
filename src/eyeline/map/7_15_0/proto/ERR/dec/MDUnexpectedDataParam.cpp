#include "MDUnexpectedDataParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDUnexpectedDataParam::construct()
{
  asn1::ber::DecoderOfSequence_T<2,1>::setField(0, asn1::_tagSEQOF,
                                                asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<2,1>::setUnkExtension(1);

}

asn1::ber::TypeDecoderAC*
MDUnexpectedDataParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDDataMissingParam::prepareAlternative : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDDataMissingParam::prepareAlternative() : undefined UId");

  if (unique_idx == 0) {
    _dExtContainer.init(getTSRule()).setValue(_value->extensionContainer.init());
    return _dExtContainer.get();
  }
  _uext.init(getTSRule()).setValue(_value->_unkExt);
  return _uext.get();
}

}}}}
