#ifndef _SCAG_PVSS_BASE_PERSSERVERRESPONSE_H
#define _SCAG_PVSS_BASE_PERSSERVERRESPONSE_H

namespace scag2 {
namespace pvss {

namespace perstypes {

enum PersServerResponseType
{
        RESPONSE_OK = 1,
        RESPONSE_ERROR,
        RESPONSE_PROPERTY_NOT_FOUND,
        RESPONSE_BAD_REQUEST,
        RESPONSE_TYPE_INCONSISTENCE,
        COMMAND_IN_PROCESS,
        RESPONSE_PROFILE_LOCKED,
        RESPONSE_NOTSUPPORT
};

const char* persServerResponse( int resp );

} // namespace perstypes

using namespace perstypes;

} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
using namespace scag2::pvss::perstypes;
}
}

#endif
