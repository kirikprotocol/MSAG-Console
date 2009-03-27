#include "GetCommand.h"
#include "ResponseTypeMatch.h"

namespace {
using namespace scag2::pvss;
class GetResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitGetResponse( GetResponse& ) /* throw (PvapException) */  {
        return true;
    }
};

GetResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

ResponseTypeMatch& GetCommand::getResponseTypeMatch() const {
    return responseMatch_;
}

} // namespace pvss
} // namespace scag2
