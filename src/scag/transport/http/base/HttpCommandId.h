#ifndef SCAG_TRANSPORT_HTTP_COMMANDID
#define SCAG_TRANSPORT_HTTP_COMMANDID

namespace scag2 {
namespace transport {
namespace http {

enum HttpCommandId {
    HTTP_REQUEST = 100,
    HTTP_RESPONSE,
    HTTP_DELIVERY
};


}}}

#endif // SCAG_TRANSPORT_HTTP_COMMANDID
