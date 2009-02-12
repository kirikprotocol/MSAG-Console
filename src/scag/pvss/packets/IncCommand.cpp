#include "IncCommand.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class IncResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitIncResponse( IncResponse& ) throw (PvapException) {
        return true;
    }
};

IncResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

ResponseTypeMatch& IncCommand::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
