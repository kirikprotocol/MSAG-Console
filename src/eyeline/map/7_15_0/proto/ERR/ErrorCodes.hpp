/* ************************************************************************* *
 * MAP OPERATIONs ERROR codes.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_ERR_CODES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_ERR_CODES_HPP

#include <inttypes.h>

namespace eyeline {
namespace map {
namespace err {

struct MAPErrorCode {
  enum Id_e {
    //-- generic errors
    me_systemFailure = 34,
    me_dataMissing = 35,
    me_unexpectedDataValue = 36,
    me_facilityNotSupported = 21,
    //-- identification and numbering errors
    me_unknownSubscriber = 1,
    me_numberChanged = 44,
    //-- subscription errors
    me_bearerServiceNotProvisioned = 10,
    me_teleserviceNotProvisioned = 11,
    //-- call handling errors
    me_absentSubscriber = 27,
    me_busySubscriber = 45,
    me_noSubscriberReply = 46,
    me_callBarred = 13,
    me_forwardingViolation = 14,
    me_cug_Reject = 15,
    me_or_NotAllowed = 48,
    //-- any time information handling errors
    me_atsi_NotAllowed = 60,
    me_informationNotAvailable = 62,
    //-- supplementary service errors
    me_illegalSS_Operation = 16,
    me_ss_NotAvailable = 18,
    me_unknownAlphabet = 71
  };

  static const char * id2name(uint32_t err_code);
};

}}}

#endif /* __EYELINE_MAP_ERR_CODES_HPP */

