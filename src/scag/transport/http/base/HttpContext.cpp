#include "HttpContext.h"

namespace scag2 { namespace transport { namespace http
{

ActionID HttpContext::actionNext[8] = {
    SEND_REQUEST,               // <- PROCESS_REQUEST
    SEND_RESPONSE,              // <- PROCESS_RESPONSE
    NOP,                        // <- PROCESS_STATUS_RESPONSE
    PROCESS_REQUEST,            // <- READ_REQUEST
    READ_RESPONSE,              // <- SEND_REQUEST
    PROCESS_RESPONSE,           // <- READ_RESPONSE
    PROCESS_STATUS_RESPONSE     // <- SEND_RESPONSE
};

const char *HttpContext::taskName[8] = {
    "Scag",             // <- PROCESS_REQUEST
    "Scag",             // <- PROCESS_RESPONSE
    "Scag",             // <- PROCESS_STATUS_RESPONSE
    "Reader",           // <- READ_REQUEST
    "Writer",           // <- SEND_REQUEST
    "Reader",           // <- READ_RESPONSE
    "Writer"            // <- SEND_RESPONSE
};

HttpContext::~HttpContext()
{
    if (user)
        delete user;
    if (site)
        delete site;

    if (command)
        delete command;
}

}}}
