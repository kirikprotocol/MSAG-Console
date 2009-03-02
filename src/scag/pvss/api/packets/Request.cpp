#include "Request.h"
#include "Response.h"
#include "ResponseTypeMatch.h"

namespace {
scag2::pvss::ResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

Request::Request() : Packet() {}

bool Request::matchResponseType( const Response& resp ) const
{
    try {
        return const_cast< Response& >(resp).visit( getResponseTypeMatch() );
    } catch ( PvapException& e ) {
        return false;
    }
}

ResponseTypeMatch& Request::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
