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
    PC_UNKNOWN,
    PC_DEL,
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
    RESPONSE_BAD_REQUEST
};

}}

#endif

