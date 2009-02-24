#include "PvssException.h"

namespace scag2 {
namespace pvss {

const char* PvssException::typeToString( Type t )
{
    switch ( t ) {
    case(UNKNOWN) : return "Unknown error";
    case(CONNECT_FAILED) : return "Can't connect/reconnect to PVSS server";
    case(NOT_CONNECTED) : return "Client is not connected to PVSS server";
    case(SERVER_BUSY) : return "Connect passed but PVSS server has rejected connection (busy)";
    case(CLIENT_BUSY) : return "Client rejects request (processing queue is full)";
    case(IO_ERROR) : return "Failed to perform IO operation. Connection corrupted";
    case(REQUEST_TIMEOUT) : return "Time to process request or IO timeout expired";
    case(UNKNOWN_RESPONSE) : return "Received response is unknown";
    case(UNEXPECTED_RESPONSE) : return "Received response is unexpected. SeqNum doesn't corresponds any sent request";
    case(BAD_REQUEST) : return "Request is bad formed";
    case(BAD_RESPONSE) : return "Response is bad formed";
    case(INVALID_KEY) : return "Provided profile key is invalid";
    case(INVALID_SCOPE) : return "Provided profile scope is invalid";
    case(CONFIG_INVALID) : return "Configuration is invalid";
    default: return "???";
    }
}

}
}
