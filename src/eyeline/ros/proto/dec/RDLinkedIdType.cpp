#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDLinkedIdType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag RDLinkedIdType::_tagPresent(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag RDLinkedIdType::_tagAbsent(asn1::ASTag::tagContextSpecific, 1);

void RDLinkedIdType::resetAlt(void)
{
  if (_alt._none) {
    _alt._none->~TypeDecoderAC();
    _alt._none = NULL;
  }
}

//Initializes ElementDecoder for this type
void RDLinkedIdType::construct(void)
{
  asn1::ber::DecoderOfChoice_T<2>::setAlternative(0, _tagPresent, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<2>::setAlternative(1, _tagAbsent, asn1::ASTagging::tagsIMPLICIT);
}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  RDLinkedIdType::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDLinkedIdType : value isn't set!");
  if (unique_idx > 1) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDLinkedIdType::prepareAlternative() : undefined UId");

  resetAlt();

  if (!unique_idx) {
    _alt._present = new (_memAlt._buf) asn1::ber::DecoderOfINTEGER(getTSRule());
    _dVal->_present = true;
    _alt._present->setValue(_dVal->_invId);
    return _alt._present;
  }
  _alt._absent = new (_memAlt._buf) asn1::ber::DecoderOfNULL(getTSRule());
  _dVal->_present = false;
  return _alt._absent;
}

}}}}

