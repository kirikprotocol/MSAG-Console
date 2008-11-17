#ifndef _SCAG_PERS_UTIL_PERSCLIENTEXCEPTION_H
#define _SCAG_PERS_UTIL_PERSCLIENTEXCEPTION_H

namespace scag2 {
namespace pers {
namespace util {

enum PersClientExceptionType 
{
        PERSCLIENTOK = 0,
        PROPERTY_NOT_FOUND,      // non-fatal exceptions
        INVALID_KEY,
        INVALID_PROPERTY_TYPE,
        TYPE_INCONSISTENCE,
        PROFILE_LOCKED,
        NONFATAL_6,
        NONFATAL_7,
        UNKNOWN_EXCEPTION = 8,   // all non-pers client exceptions are converted into this one
        CANT_CONNECT,
        SERVER_BUSY,
        UNKNOWN_RESPONSE,
        SEND_FAILED,
        READ_FAILED,
        TIMEOUT,
        NOT_CONNECTED,
        BAD_RESPONSE,
        SERVER_ERROR,
        BAD_REQUEST,
        BATCH_ERROR,
        COMMAND_NOTSUPPORT,
        CLIENT_BUSY
};

static const char* strs[] = {
        "Ok",
        "Property not found",
        "Invalid key",
        "Invalid property type(should be int or date)",
        "Types inconsistence",
        "Profile locked",
        "Non-fatal #6",
        "Non-fatal #7",
        "Unknown exception",
        "Cant connect to persserver",
        "Server busy",
        "Unknown server response",
        "Send failed",
        "Read failed",
        "Read/write timeout",
        "Not connected",
        "Bad response",
        "Server error",
        "Bad request",
        "Batch prepare error",
        "Command Not Supported",
        "Client busy"
};

class PersClientException
{
public:
    PersClientException(PersClientExceptionType e) { et = e; };
    PersClientExceptionType getType() const { return et; };
    const char* what() const { return strs[et]; };
protected:
    PersClientExceptionType et;
};

}
}
}

#endif /* !_SCAG_PERS_UTIL_PERSCLIENTEXCEPTION_H */
