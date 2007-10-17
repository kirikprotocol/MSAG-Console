/* $Id$ */

#ifndef SCAG_PERS_TYPES
#define SCAG_PERS_TYPES

namespace scag { namespace pers {

enum ProfileType{
    PT_UNKNOWN,
    PT_ABONENT,
    PT_OPERATOR,
    PT_PROVIDER,
    PT_SERVICE
};

enum PersCmd{
    PC_DEL = 1,
    PC_SET,
    PC_GET,
    PC_INC,
    PC_INC_MOD,
    PC_PING,
	PC_BATCH
};

namespace CentralPersCmd{
    enum{
        UNKNOWN,
        GET,
        ACK,
        PROFILE,
        DONE,
        CHECK_OWN,
        LOGIN
    };
};

enum PersServerResponseType{
    RESPONSE_OK = 1,
    RESPONSE_ERROR,
    RESPONSE_PROPERTY_NOT_FOUND,
    RESPONSE_BAD_REQUEST
};

}}

#endif

