#include "StatusType.h"

namespace scag2 {
namespace pvss {

const char* StatusType::statusToString( uint8_t stat )
{
#define STATSTRING(x) case (x) : return #x
    switch (stat) {
        STATSTRING(OK);
        STATSTRING(PROPERTY_NOT_FOUND);
        STATSTRING(TYPE_INCONSISTENCE);
        STATSTRING(INVALID_KEY);
        STATSTRING(INVALID_SCOPE);

        STATSTRING(BAD_REQUEST);
        STATSTRING(BAD_RESPONSE);
        STATSTRING(UNKNOWN_RESPONSE);
        STATSTRING(UNEXPECTED_RESPONSE);

        STATSTRING(IO_ERROR);
        STATSTRING(NOT_SUPPORTED);
        STATSTRING(SERVER_SHUTDOWN);
        STATSTRING(SERVER_BUSY);
        STATSTRING(REQUEST_TIMEOUT);
        STATSTRING(CONNECT_FAILED);
        STATSTRING(NOT_CONNECTED);
        STATSTRING(CLIENT_BUSY);
        STATSTRING(CONFIG_INVALID);

        STATSTRING(UNKNOWN);
#undef STATSTRING
    default : return "???";
    }
}


const char* StatusType::statusMessage( uint8_t stat )
{
    switch (stat) {
    case (OK)                  : return "Response Ok";
    case (PROPERTY_NOT_FOUND)  : return "Property not found";
    case (TYPE_INCONSISTENCE)  : return "Type inconsistence";
    case (INVALID_KEY)         : return "Provided profile key is invalid";
    case (INVALID_SCOPE)       : return "Provided profile scope is invalid";

    case (BAD_REQUEST)         : return "Request is bad formed";
    case (BAD_RESPONSE)        : return "Response is bad formed";
    case (UNKNOWN_RESPONSE)    : return "Received response is unknown";
    case (UNEXPECTED_RESPONSE) : return "Received response has unexpected seqNum";

    case (IO_ERROR)            : return "Failed to perform IO operation";
    case (NOT_SUPPORTED)       : return "Feature is not supported (yet?)";
    case (SERVER_SHUTDOWN)     : return "Server is shutting down";
    case (SERVER_BUSY)         : return "Server busy";
    case (REQUEST_TIMEOUT)     : return "Request is timed out";
    case (CONNECT_FAILED)      : return "Can't connect/reconnect to PVSS server";
    case (NOT_CONNECTED)       : return "Client is not connected to PVSS server";
    case (CLIENT_BUSY)         : return "Client queue is full (request rejected)";
    case (CONFIG_INVALID)      : return "Configuration is invalid";
    case (UNKNOWN)             : return "Unknown error";
    default: return "???";
    }
}

} // namespace pvss
} // namespace scag2
