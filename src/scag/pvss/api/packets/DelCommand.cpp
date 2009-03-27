#include "DelCommand.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class DelResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitDelResponse( DelResponse& ) /* throw (PvapException) */  {
        return true;
    }
};

DelResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

ResponseTypeMatch& DelCommand::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
