#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "BillingManager.h"
#include "scag/bill/ewallet/Request.h"
#include "scag/bill/ewallet/Response.h"
#include "scag/bill/ewallet/Exception.h"

using namespace scag2::bill;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< BillingManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

/// BillingManager is required for sessionmanager.
inline unsigned GetLongevity( BillingManager* ) { return 251; }

}

namespace scag2 {
namespace bill {

EwalletCallParams::EwalletCallParams( lcm::LongCallContext* lcmCtx ) :
lcmCtx_(lcmCtx), status_(ewallet::Status::OK) {}


void EwalletCallParams::handleResponse( std::auto_ptr< ewallet::Request > request, 
                                            std::auto_ptr< ewallet::Response > response )
{
    if ( response.get() && response->getStatus() != ewallet::Status::OK ) {
        setStatus( response->getStatus(), "text message to be filled oneday" );
    } else {
        setResponse( *response.get() );
    }
    continueExecution();
}


void EwalletCallParams::handleError( std::auto_ptr< ewallet::Request > request,
                                     const ewallet::Exception& error )
{
    setStatus( error.getStatus(), error.what() );
    continueExecution();
}


void EwalletCallParams::continueExecution()
{
    if ( lcmCtx_ ) lcmCtx_->initiator->continueExecution(lcmCtx_,false);
}


void EwalletOpenCallParams::setResponse( ewallet::Response& resp )
{
    // status is ok
    // FIXME
}


BillingManager& BillingManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("BillingManager not inited");
    }
    return Single::Instance();
}

BillingManager::BillingManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

BillingManager::~BillingManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace bill
} // namespace scag2
