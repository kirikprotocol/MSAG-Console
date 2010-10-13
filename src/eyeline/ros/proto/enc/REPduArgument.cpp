#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/REPduArgument.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

void REPduArgument::setValue(const PDUArgument & use_val) /*throw(std::exception)*/
{
  if (use_val._kind & PDUArgument::asvTSyntax)
    asn1::ber::EncoderOfASType::setValue(use_val._tsEnc);
  else {
    asn1::ber::TypeEncoderAC * pEnc = 
      static_cast<asn1::ber::TypeEncoderAC *>(use_val._asType->getEncoder(getTSRule()));
    asn1::ber::EncoderOfASType::setValue(*pEnc);
  }
}

}}}}

