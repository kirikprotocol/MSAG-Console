#include "AuthRequest.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class AuthResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitAuthResponse( AuthResponse& resp ) throw (PvapException) {
        return true;
    }
};

AuthResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

std::string AuthRequest::toString() const
{
    std::string rv;
    rv.reserve(200);
    char buf[30];
    sprintf( buf," version=%u login=", unsigned(protocolVersion_) );
    rv = AbstractNonProfileRequest::toString() + buf + login_ +
        " password=" + password_ +
        " name=" + name_;
    return rv;
}

ResponseTypeMatch& AuthRequest::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
