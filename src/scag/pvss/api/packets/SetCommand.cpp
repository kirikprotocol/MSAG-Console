#include "SetCommand.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class SetResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitSetResponse( SetResponse& ) throw (PvapException) {
        return true;
    }
};

SetResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

ResponseTypeMatch& SetCommand::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
