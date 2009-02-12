#include "PingRequest.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class PingResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitPingResponse( PingResponse& resp ) throw (PvapException) {
        return true;
    }
};

PingResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

ResponseTypeMatch& PingRequest::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
