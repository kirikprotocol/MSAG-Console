#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "BillingManager.h"
#include "scag/bill/ewallet/Request.h"
#include "scag/bill/ewallet/Response.h"
#include "scag/bill/ewallet/OpenResp.h"
#include "scag/bill/ewallet/CheckResp.h"
#include "scag/bill/ewallet/TransferResp.h"
#include "scag/bill/ewallet/InfoResp.h"
#include "scag/bill/ewallet/Exception.h"

using namespace scag2::bill;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< BillingManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;
Mutex logMutex;

/// BillingManager is required for sessionmanager.
inline unsigned GetLongevity( BillingManager* ) { return 251; }

}

namespace scag2 {
namespace bill {

smsc::logger::Logger* BillCallParams::log_ = 0;

BillCallParams::BillCallParams()
{
    if ( ! log_ ) {
        MutexGuard mg(logMutex);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("bill.param");
    }
}


EwalletCallParams::EwalletCallParams( bool transit, lcm::LongCallContext* lcmCtx ) :
lcmCtx_(lcmCtx), registrator_(0), transit_(transit), transId_(0), status_(ewallet::Status::OK) {}


void EwalletCallParams::handleResponse( std::auto_ptr< ewallet::Request > request, 
                                        std::auto_ptr< ewallet::Response > response )
{
    smsc_log_debug(log_,"ewallet handle response: req=%s resp=%s",
                   request->toString().c_str(), response->toString().c_str() );
    if ( !response.get() ) {
        setStatus( ewallet::Status::BAD_RESPONSE, "response is missing");
    } else if ( response->getStatus() != ewallet::Status::OK ) {
        setStatus( response->getStatus(), ewallet::Status::statusToString(response->getStatus()) );
    } else {
        setResponse( *response.get() );
    }
    if (registrator_) registrator_->processAsyncResult(*this);
    continueExecution();
}


void EwalletCallParams::handleError( std::auto_ptr< ewallet::Request > request,
                                     const ewallet::Exception& error )
{
    smsc_log_debug(log_,"ewallet handle error: req=%s err=%s stat=%u",
                   request->toString().c_str(), error.what(), unsigned(error.getStatus()) );
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
    ewallet::OpenResp& oResp = static_cast< ewallet::OpenResp& >(resp);
    setTransId( oResp.getTransId() );
    setResultAmount( oResp.getAmount() );
    setChargeThreshold( oResp.getChargeThreshold() );
}


void EwalletCloseCallParams::setResponse( ewallet::Response& resp )
{
    // status is ok
    // delete the transaction (will be done in billing manager and session)
}


bool EwalletCloseCallParams::isCommit() const
{
    return lcmCtx_->callCommandId == lcm::BILL_COMMIT;
}


void EwalletCheckCallParams::setResponse( ewallet::Response& resp )
{
    ewallet::CheckResp& cResp = static_cast< ewallet::CheckResp& >(resp);
    transStatus_ = cResp.getTransStatus();
    txAmount_ = -cResp.getAmount();
    txEndDate_ = cResp.getEnddate();
}


void EwalletTransferCallParams::setResponse( ewallet::Response& resp )
{
    ewallet::TransferResp& cResp = static_cast< ewallet::TransferResp& >(resp);
    smsc_log_debug(log_,"FIXME: transfer::setResponse invoked");
}


void EwalletInfoCallParams::setResponse( ewallet::Response& resp )
{
    // status is ok
    ewallet::InfoResp& oResp = static_cast< ewallet::InfoResp& >(resp);
    setResultAmount( oResp.getAmount() );
    setChargeThreshold( oResp.getChargeThreshold() );
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
