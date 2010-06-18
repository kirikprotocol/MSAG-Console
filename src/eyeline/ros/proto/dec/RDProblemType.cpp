#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDProblemType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag
  RDProblemType::_tagGeneralProblem(asn1::ASTag::tagContextSpecific, 0);

const asn1::ASTag
  RDProblemType::_tagInvokeProblem(asn1::ASTag::tagContextSpecific, 1);

const asn1::ASTag
  RDProblemType::_tagResultProblem(asn1::ASTag::tagContextSpecific, 2);

const asn1::ASTag
  RDProblemType::_tagErrorProblem(asn1::ASTag::tagContextSpecific, 3);

//Initializes ElementDecoder for this type
//NOTE: alternative indexes match values of ros::RejectProblem::ProblemKind_e
void RDProblemType::construct(void)
{
  asn1::ber::DecoderOfChoice_T<4>::setAlternative(0, _tagGeneralProblem, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<4>::setAlternative(1, _tagInvokeProblem, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<4>::setAlternative(2, _tagResultProblem, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<4>::setAlternative(3, _tagErrorProblem, asn1::ASTagging::tagsIMPLICIT);
}

void RDProblemType::initAlt(const asn1::ASTag & problem_tag)
{
  _pDec = new (_memAlt._buf) asn1::ber::DecoderOfINTEGER(problem_tag,
                                  asn1::ASTagging::tagsIMPLICIT, getTSRule());
  _pDec->setValue(_tmpVal);
}


// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  RDProblemType::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDProblemType : value isn't set!");
  if (unique_idx > 3) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDProblemType::prepareAlternative() : undefined UId");

  resetAlt();

  switch (unique_idx) {
  case 3:
    initAlt(_tagErrorProblem); break;
  case 2:
    initAlt(_tagResultProblem); break;
  case 1:
    initAlt(_tagInvokeProblem); break;
  default: //case 0:
    initAlt(_tagGeneralProblem);
  }
  return _pDec;
}

//Perfoms actions finalizing alternative decoding
void RDProblemType::markDecodedAlternative(uint16_t unique_idx) /*throw(throw(std::exception)) */
{
  _dVal->setProblem(static_cast<ros::RejectProblem::ProblemKind_e>((uint8_t)unique_idx), _tmpVal);
}


}}}}
