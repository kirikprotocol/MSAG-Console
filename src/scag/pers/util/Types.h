/* $Id$ */

#ifndef SCAG_PERS_TYPES
#define SCAG_PERS_TYPES

namespace scag { namespace pers { namespace util {

namespace perstypes {

enum ProfileType{
    PT_UNKNOWN,
    PT_ABONENT,
    PT_OPERATOR,
    PT_PROVIDER,
    PT_SERVICE
};

const char* persProfileType(ProfileType pt);

enum PersCmd{
    PC_UNKNOWN = 0,
    PC_DEL,
    PC_SET,
    PC_GET,
    PC_INC,
    PC_INC_MOD,
    PC_PING,
    PC_BATCH,
    PC_TRANSACT_BATCH,
    PC_INC_RESULT,
    PC_MTBATCH,
    PC_BIND_ASYNCH
};

const char* persCmdName(PersCmd c);

namespace CentralPersCmd{
    enum{
        UNKNOWN,
        GET_PROFILE,
        PROFILE_RESP,
        DONE,
        DONE_RESP,
        CHECK_OWN, //if transaction not complite
        CHECK_OWN_RESP,
        LOGIN,
        PING,
        PING_OK
    };
};

enum PersServerResponseType{
  RESPONSE_OK = 1,
  RESPONSE_ERROR,
  RESPONSE_PROPERTY_NOT_FOUND,
  RESPONSE_BAD_REQUEST,
  RESPONSE_TYPE_INCONSISTENCE,
  COMMAND_IN_PROCESS,
  RESPONSE_PROFILE_LOCKED,
  RESPONSE_NOTSUPPORT
};

extern const char* RESPONSE_TEXT[];

} // perstypes

using namespace perstypes;

}//util
}//pers
}//scag


namespace scag2 {
namespace pers {
namespace util {
using namespace scag::pers::util::perstypes;
}
}
}

#endif

