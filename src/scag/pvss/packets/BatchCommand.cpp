#include "BatchCommand.h"
#include "ResponseTypeMatch.h"
#include "BatchRequestComponent.h"

namespace {
using namespace scag2::pvss;
class BatchResponseTypeMatch : public ResponseTypeMatch
{
public:
    virtual bool visitBatchResponse( BatchResponse& ) throw (PvapException) {
        return true;
    }
};

BatchResponseTypeMatch responseMatch_;
}

namespace scag2 {
namespace pvss {

bool BatchCommand::isValid() const 
{
    if ( batchContent_.empty() ) return false;
    for ( std::vector< BatchRequestComponent* >::const_iterator i = batchContent_.begin();
          i != batchContent_.end();
          ++i ) {
        if ( ! (*i)->isValid() ) return false;
    }
    return true;
}

std::string BatchCommand::toString() const 
{
    std::string rv(AbstractCommand::toString() + " [");
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
    for ( std::vector< BatchRequestComponent* >::iterator i = batchContent_.begin();
          i != batchContent_.end();
          ++i ) {
        delete *i;
    }
    batchContent_.clear();
}
    
ResponseTypeMatch& BatchCommand::getResponseTypeMatch() const {
    return responseMatch_;
}

BatchCommand::BatchCommand( const BatchCommand& cmd ) :
AbstractCommand(cmd),
transactional_(cmd.transactional_)
{
    for ( std::vector< BatchRequestComponent* >::const_iterator i = cmd.batchContent_.begin();
          i != cmd.batchContent_.end();
          ++i ) {
        batchContent_.push_back( (*i)->clone() );
    }
}

} // namespace pvss
} // namespace scag2
