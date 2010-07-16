#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/ERR/ErrorCodes.hpp"

namespace eyeline {
namespace map {
namespace err {

const char * MAPErrorCode::id2name(uint32_t err_code)
{
  switch (err_code) {
  /* -- generic errors */
  case MAPErrorCode::me_systemFailure: return "systemFailure";
  case MAPErrorCode::me_dataMissing: return "dataMissing";
  case MAPErrorCode::me_unexpectedDataValue: return "unexpectedDataValue";
  case MAPErrorCode::me_facilityNotSupported: return "facilityNotSupported";
  /* -- identification and numbering errors */
  case MAPErrorCode::me_unknownSubscriber: return "unknownSubscriber";
  case MAPErrorCode::me_numberChanged: return "numberChanged";
  /* -- subscription errors */
  case MAPErrorCode::me_bearerServiceNotProvisioned: return "bearerServiceNotProvisioned";
  case MAPErrorCode::me_teleserviceNotProvisioned: return "teleserviceNotProvisioned";
  /* -- call handling errors */
  case MAPErrorCode::me_absentSubscriber: return "absentSubscriber";
  case MAPErrorCode::me_busySubscriber: return "busySubscriber";
  case MAPErrorCode::me_noSubscriberReply: return "noSubscriberReply";
  case MAPErrorCode::me_callBarred: return "callBarred";
  case MAPErrorCode::me_forwardingViolation: return "forwardingViolation";
  case MAPErrorCode::me_cug_Reject: return "cug-Reject";
  case MAPErrorCode::me_or_NotAllowed: return "or-NotAllowed";
  /* -- any time information handling errors */
  case MAPErrorCode::me_atsi_NotAllowed: return "atsi-NotAllowed";
  case MAPErrorCode::me_informationNotAvailable: return "informationNotAvailable";
  /* -- supplementary service errors */
  case MAPErrorCode::me_illegalSS_Operation: return "illegalSS-Operation";
  case MAPErrorCode::me_ss_NotAvailable: return "ss-NotAvailable";
  case MAPErrorCode::me_unknownAlphabet: return "unknownAlphabet";
  default:; 
  }
  return "unknown MAP Operation error code";
}

}}}

