#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/dec/RDLinkedIdType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag RDLinkedIdType::_tagPresent(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag RDLinkedIdType::_tagAbsent(asn1::ASTag::tagContextSpecific, 1);

/* LinkedIdType is defined in IMPLICIT tagging environment as follow:
  LinkedIdType ::=  CHOICE {
      present  [0] IMPLICIT InvokeIdType,
      absent   [1] IMPLICIT NULL
  }
*/
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

  if (!unique_idx) {
    _alt.present().init(getTSRule()).setValue(_dVal->_invId);
    return _alt.present().get();
  }
  _alt.absent().init(getTSRule());
  return _alt.absent().get();
}

//Perfoms actions finalizing alternative decoding
void RDLinkedIdType::markDecodedAlternative(uint16_t unique_idx)
  /*throw(throw(std::exception)) */
{
  if (!unique_idx)
    _dVal->_present = true;
  else
    _dVal->_present = false;
}


}}}}

