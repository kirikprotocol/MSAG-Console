#include "AbstractProfileRequest.h"
#include "ProfileRequest.h"
#include "ProfileCommandVisitor.h"
#include "DelCommand.h"
#include "GetCommand.h"
#include "SetCommand.h"
#include "IncCommand.h"
#include "IncModCommand.h"
#include "BatchCommand.h"

namespace {

using namespace scag2::pvss;

struct PRMaker : public ProfileCommandVisitor
{
    virtual bool visitDelCommand( DelCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    virtual bool visitSetCommand( SetCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    virtual bool visitGetCommand( GetCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    virtual bool visitIncCommand( IncCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    virtual bool visitIncModCommand( IncModCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    virtual bool visitBatchCommand( BatchCommand& cmd ) throw(PvapException) {
        req = createProfileRequest(&cmd);
        return true;
    }
    AbstractProfileRequest* req;
};

}

namespace scag2 {
namespace pvss {

AbstractProfileRequest* AbstractProfileRequest::create(AbstractCommand* cmd)
{
    if ( ! cmd ) return 0;
    PRMaker maker;
    cmd->visit(maker);
    return maker.req;
}


AbstractProfileRequest::~AbstractProfileRequest()
{
    if ( context_ ) smsc_log_warn( log_, "request %p has extra context %p at dtor", this, context_ );
}


void AbstractProfileRequest::setExtraContext( void* context )
{
    if ( context && context_ ) {
        smsc_log_warn( log_, "request %p has replaced ctx %p with %p", this, context_, context );
    }
    context_ = context;
}
    
} // namespace pvss
} // namespace scag2
