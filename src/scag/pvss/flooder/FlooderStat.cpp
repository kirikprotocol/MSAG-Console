#include "FlooderStat.h"
#include "FlooderThread.h"

namespace scag2 {
namespace pvss {
namespace flooder {

FlooderStat::FlooderStat( const FlooderConfig& config,
                          core::client::Client& client ) :
log_(0),
requestsPerSecond_(config.getSpeed()),
requested_(config.getGetSetCount()),
doneTime_(0),
timeout_(100),
stopped_(false),
config_(config),
generator_(),
client_(client),
tp_(new smsc::core::threads::ThreadPool)
{
    log_ = smsc::logger::Logger::getInstance("flooder");
}

FlooderStat::~FlooderStat()
{
    shutdown(); 
}


void FlooderStat::handleResponse( std::auto_ptr< Request > request, std::auto_ptr< Response > response )
{
    smsc_log_debug(log_,"response got: req=%s resp=%s", request->toString().c_str(), response->toString().c_str() );
    {
        MutexGuard mg(mon_);
        ++total_.responses;
        ++last_.responses;
        checkTime();
    }
}


void FlooderStat::handleError( const PvssException& e, std::auto_ptr<Request> request )
{
    smsc_log_debug(log_,"error got: req=%s except=%s", request->toString().c_str(), e.what() );
    {
        MutexGuard mg(mon_);
        ++total_.errors;
        ++last_.errors;
        checkTime();
    }
}


void FlooderStat::requestCreated()
{
    MutexGuard mg(mon_);
    // if ( total_.requests == 0 ) mon_.notifyAll();
    ++total_.requests;
    ++last_.requests;
    checkTime();
    // check 
    if ( requested_ != 0 && total_.requests >= requested_ ) {
        mon_.notifyAll();
    }
}


void FlooderStat::adjustSpeed()
{
    /*
    if ( total_.requests == 0 ) {
        // not started
        MutexGuard mg(mon_);
        mon_.wait(100);
        return;
    }
     */

    if ( total_.requests % 4 == 0 && total_.requests > 50 && total_.elapsedTime > 100 ) {
        // try to adjust speed
        unsigned expected = (total_.elapsedTime * requestsPerSecond_ / 1000);
        smsc_log_debug(log_,"adjustSpeed: requests=%d time=%d expected=%d",
                       total_.requests, unsigned(total_.elapsedTime), expected );
        if ( total_.requests > expected ) {
            MutexGuard mg(mon_);
            // recheck
            expected = total_.elapsedTime * requestsPerSecond_ / 1000;
            if ( total_.requests > expected ) {
                // waiting time
                util::msectime_type waitTime = (total_.requests - expected) * 1000 / requestsPerSecond_;
                if ( waitTime > 10 ) {
                    mon_.wait(waitTime);
                }
            }
        }
    }
}


void FlooderStat::waitUntilProcessed()
{
    MutexGuard mg(mon_);
    if ( stopped_ ) return;
    if ( requested_ == 0 ) {
        mon_.wait();
    } else {
        while ( total_.requests < requested_ ) {
            if ( stopped_ ) return;
            mon_.wait(1000);
        }
        util::msectime_type currentTime;
        while ( total_.responses+total_.errors < total_.requests ) {
            if ( stopped_ ) return;
            currentTime = util::currentTimeMillis();
            if ( doneTime_+timeout_ < currentTime ) {
                break;
            }
            mon_.wait( doneTime_ + timeout_ - currentTime );
        }
    }
}


void FlooderStat::startup() throw (exceptions::IOException)
{
    stopped_ = false;

    // FIXME: take params from config
    generator_.randomizeProfileKeys( ".1.1.791%08d", config_.getAddressesCount());
    generator_.addPropertyPattern(0,new Property("test0",101,FIXED,time(0)+1234,123));
    generator_.addPropertyPattern(1,new Property("test1","хелло",R_ACCESS,time(0)+3456,17));
    generator_.parseCommandPatterns("s0s1b2g0g1b2d0d1");

    for ( int i = 0; i < config_.getFlooderThreadCount(); ++i ) {
        util::WatchedThreadedTask* task;
        if ( config_.getAsyncMode() ) {
            task = new AsyncFlooderThread(*this);
        } else {
            task = new SyncFlooderThread(*this);
        }
        tasks_.Push(task);
        tp_->startTask(task,false);
    }
}


void FlooderStat::shutdown()
{
    if ( stopped_ ) return;
    {
        MutexGuard mg(mon_);
        if ( stopped_ ) return;
        smsc_log_info(log_,"Flooder stat is shutting down...");
        stopped_ = true;
        mon_.notifyAll();
    }
    tp_->stopNotify();
    for ( int i = 0; i < tasks_.Count(); ++i ) {
        tasks_[i]->waitUntilReleased();
    }
    tp_->shutdown();
    for ( int i = 0; i < tasks_.Count(); ++i ) {
        delete tasks_[i];
    }
    tasks_.Empty();
    // waitUntilReleased();
    tp_.reset(0);
    smsc_log_info(log_,"Flooder stat is shutdowned");
}


void FlooderStat::checkTime()
{
    util::msectime_type currentTime = util::currentTimeMillis();
    total_.checkTime( currentTime );
    if ( last_.checkTime( currentTime ) ) {
        // a new statistics chunk is filled
        previous_ = last_;
        last_.reset();
        smsc_log_info(log_,"total: req=%u resp=%u err=%u speed=%f req/s",
                      total_.requests, total_.responses, total_.errors,
                      total_.requests/(double(total_.elapsedTime)/1000));
        smsc_log_info(log_,"last %d sec: req=%u resp=%u err=%u speed=%f req/s",
                      unsigned(previous_.elapsedTime/1000),
                      previous_.requests, previous_.responses, previous_.errors,
                      previous_.requests/(double(previous_.elapsedTime)/1000));
        if ( previous_.requests == 0 ) {
            smsc_log_warn(log_,"there were no requests during last %u msec, stopping",
                          unsigned(previous_.elapsedTime/1000));
            stopped_ = true;
            mon_.notifyAll();
        }
    }
}

} // namespace flooder
} // namespace pvss
} // namespace scag2
