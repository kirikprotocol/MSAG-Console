#ifndef _SCAG_PERS_UTIL_PERSCLIENTEXCEPTION_H
#define _SCAG_PERS_UTIL_PERSCLIENTEXCEPTION_H

namespace scag2 {
namespace pers {
namespace util {

enum PersClientExceptionType {
  CANT_CONNECT = 1,
  SERVER_BUSY,
  UNKNOWN_RESPONSE,
  SEND_FAILED,
  READ_FAILED,
  TIMEOUT,
  NOT_CONNECTED,
  BAD_RESPONSE,
  SERVER_ERROR,
  PROPERTY_NOT_FOUND,
  INVALID_KEY,
  INVALID_PROPERTY_TYPE,
  BAD_REQUEST,
  TYPE_INCONSISTENCE,
  BATCH_ERROR,
  PROFILE_LOCKED,
  COMMAND_NOTSUPPORT,
  CLIENT_BUSY
};

static const char* strs[] = {
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
  "Property not found",
  "Invalid key",
  "Invalid property type(should be int or date)",
  "Bad request",
  "Types inconsistence",
  "Batch prepare error",
  "Profile locked",
  "Command Not Supports",
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
