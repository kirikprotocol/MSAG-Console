#include "Request.h"
#include "RequestVisitor.h"

namespace {

using namespace scag2::bill::ewallet;

struct PingVisitor : public RequestVisitor
{
public:
    virtual bool visitPing( Ping& ) { return true; }
    virtual bool visitAuth( Auth& ) { return false; }
    virtual bool visitOpen( Open& ) { return false; }
    virtual bool visitCommit( Commit& ) { return false; }
    virtual bool visitRollback( Rollback& ) { return false; }
};
PingVisitor pingVisitor;
}

namespace scag2 {
namespace bill {
namespace ewallet {

bool Request::isPing( const Request& req )
{
    return const_cast<Request&>(req).visit(pingVisitor);
}

} // namespace ewallet
} // namespace bill
} // namespace scag2
