/* ************************************************************************** *
 * SUA message identifiers definition.
 * ************************************************************************** */
#ifndef __EYELINE_SUA_LIBSUA_TYPES_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SUA_LIBSUA_TYPES_HPP__

# include <inttypes.h>

namespace eyeline {
namespace sua {
namespace libsua {

typedef uint32_t message_type_t;

struct SUAMessageId {
  enum MsgCode_e { UNKNOWN_MSGCODE = 0,
    BIND_MSGCODE = 0x01, BIND_CONFIRM_MSGCODE = 0x81, UNBIND_MSGCODE = 0x02,
    N_UNITDATA_REQ_MSGCODE = 0x03, N_UNITDATA_IND_MSGCODE = 0x04,
    N_NOTICE_IND_MSGCODE = 0x05, N_PCSTATE_IND_MSGCODE = 0x06,
    N_STATE_IND_MSGCODE = 0x07, N_COORD_IND_MSGCODE = 0x08 };
};

} //libsua
} //sua
} //eyeline

#endif /* __EYELINE_SUA_LIBSUA_TYPES_HPP__ */

