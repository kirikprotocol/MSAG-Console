#include "BatchCommand.h"
// #include "ResponseTypeMatch.h"
#include "BatchRequestComponent.h"

/*
namespace {
using namespace scag2::pvss;
class BatchResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitBatchResponse( BatchResponse& ) { // throw (PvapException)
        return true;
    }
};

BatchResponseTypeMatch responseMatch_;
}
 */

namespace scag2 {
namespace pvss {

bool BatchCommand::isValid( PvssException* exc ) const
{
    CHECKMAGTC;
    if ( batchContent_.empty() ) {
        if ( exc ) { *exc = PvssException("batch has no commands", PvssException::BAD_REQUEST); }
        return false;
    }
    for ( std::vector< BatchRequestComponent* >::const_iterator i = batchContent_.begin();
          i != batchContent_.end();
          ++i ) {
        if ( ! (*i)->isValid(exc) ) return false;
    }
    return true;
}

std::string BatchCommand::toString() const 
{
    CHECKMAGTC;
    char buf[12];
    snprintf(buf,sizeof(buf)," trans=%d [",transactional_ ? 1 : 0);
    std::string rv(ProfileCommand::toString() + buf);
    bool comma = false;
    for ( std::vector< BatchRequestComponent* >::const_iterator i = batchContent_.begin();
          i != batchContent_.end();
          ++i ) {
        if ( comma ) rv += ", ";
        else comma = true;
        rv += (*i)->toString();
    }
    rv += "]";
    return rv;
}

void BatchCommand::clear() 
{
    CHECKMAGTC;
    for ( std::vector< BatchRequestComponent* >::iterator i = batchContent_.begin();
          i != batchContent_.end();
          ++i ) {
        delete *i;
    }
    batchContent_.clear();
}
    
/*
ResponseTypeMatch& BatchCommand::getResponseTypeMatch() const {
    return responseMatch_;
}
 */

BatchCommand::BatchCommand( const BatchCommand& cmd ) :
ProfileCommand(cmd),
transactional_(cmd.transactional_)
{
    for ( std::vector< BatchRequestComponent* >::const_iterator i = cmd.batchContent_.begin();
          i != cmd.batchContent_.end();
          ++i ) {
        batchContent_.push_back( (*i)->clone() );
    }
    initLog();
}

} // namespace pvss
} // namespace scag2
