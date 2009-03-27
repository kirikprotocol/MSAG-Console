#include "FlooderStat.h"
#include "FlooderThread.h"
#include "scag/util/HRTimer.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"
#include "scag/util/histo/HistoLogger.h"

namespace scag2 {
namespace pvss {
namespace flooder {

FlooderStat::FlooderStat( const FlooderConfig& config,
                          core::client::Client& client ) :
log_(0),
requestsPerSecond_(config.getSpeed()),
requested_(config.getRequested()),
doneTime_(0),
timeout_(100),
stopped_(false),
total_(0),
last_(accumulationTime),
config_(config),
generator_(),
client_(client),
tp_(new smsc::core::threads::ThreadPool),
totalHisto_(binScale,histoBins,minTime),
lastHisto_(binScale,histoBins,minTime),
lastHistoCreateTime_(util::currentTimeMillis())
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
    // checking timer
    unsigned processTime; // in usec
    if ( request.get() ) {
        AbstractProfileRequest* req = static_cast<AbstractProfileRequest*>(request.get());
        if ( req->getExtraContext() ) {
            // timer has been setup
            util::HRTimer* timer = static_cast<util::HRTimer*>(req->getExtraContext());
            req->setExtraContext(0);
            processTime = unsigned(timer->get()/1000);
            delete timer;
        } else {
            processTime = 0;
        }
    } else {
        processTime = 0;
    }
    {
        MutexGuard mg(mon_);
        ++total_.responses;
        ++last_.responses;
        if ( processTime ) {
            totalHisto_.fill( processTime );
            lastHisto_.fill( processTime );
        }
        checkTime();
    }
}


void FlooderStat::handleError( const PvssException& e, std::auto_ptr<Request> request )
{
    smsc_log_debug(log_,"error got: req=%s except=%s", request->toString().c_str(), e.what() );
    if ( request.get() ) {
        AbstractProfileRequest* req = static_cast<AbstractProfileRequest*>(request.get());
        if ( req->getExtraContext() ) {
            // simply delete extra context
            util::HRTimer* timer = static_cast<util::HRTimer*>(req->getExtraContext());
            req->setExtraContext(0);
            delete timer;
        }
    }
    {
        MutexGuard mg(mon_);
        ++total_.errors;
        ++last_.errors;
        checkTime();
    }
}


void FlooderStat::requestCreated( Request* req )
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
    // do we need a timer?
    static unsigned counter = 0;
    if ( requestsPerSecond_ <= 100 || ++counter % 10 == 0 ) {
        util::HRTimer* timer = new util::HRTimer();
        timer->mark();
        static_cast< AbstractProfileRequest* >(req)->setExtraContext(timer);
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

    if ( total_.requests % 4 == 0 && total_.requests > 5 && total_.elapsedTime > 10 ) {
        // try to adjust speed
        unsigned expected = unsigned(total_.elapsedTime * requestsPerSecond_ / 1000);
        smsc_log_debug(log_,"adjustSpeed: requests=%d time=%d expected=%d",
                       total_.requests, unsigned(total_.elapsedTime), expected );
        if ( total_.requests > expected ) {
            MutexGuard mg(mon_);
            // recheck
            expected = unsigned(total_.elapsedTime * requestsPerSecond_ / 1000);
            if ( total_.requests > expected ) {
                // waiting time
                int waitTime = int((total_.requests - expected) * 1000 / requestsPerSecond_);
                if ( waitTime > 10 ) {
                    smsc_log_debug(log_,"adjustSpeed waiting %u msec", waitTime );
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
            mon_.wait( int(doneTime_ + timeout_ - currentTime) );
        }
    }
}


void FlooderStat::init() throw (exceptions::IOException)
{
    const std::vector< std::string >& patterns = config_.getPropertyPatterns();
    if ( patterns.size() <= 0 ) throw exceptions::IOException("too few property patterns configured");

    smsc_log_info(log_,"shuffling %u addresses", unsigned(config_.getAddressesCount()));
    // FIXME: take params from config
    generator_.randomizeProfileKeys( config_.getAddressFormat().c_str(), config_.getAddressesCount());
    for ( std::vector< std::string >::const_iterator i = patterns.begin();
          i != patterns.end(); ++i ) {
        std::auto_ptr<Property> p( new Property );
        p->fromString( *i );
        generator_.addPropertyPattern( unsigned(i - patterns.begin()), p.release() );
    }
    generator_.parseCommandPatterns( config_.getCommands() );
}


void FlooderStat::startup()
{
    stopped_ = false;
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
        smsc_log_info(log_,"total: %s", total_.toString().c_str());
        smsc_log_info(log_,"last %u sec: %s", unsigned(previous_.accumulationTime/1000), previous_.toString().c_str());
        if ( lastHisto_.getTotal() >= 1000 ) {
            // statistics is enough
            scag_plog_info(si,log_);
            util::histo::HistoLogger hl(si);
            hl.printHisto(totalHisto_, "total");
            hl.printHisto(lastHisto_, "last %u sec", unsigned((currentTime - lastHistoCreateTime_)/1000) );
            lastHisto_.reset();
            lastHistoCreateTime_ = currentTime;
        }

        if ( previous_.requests == 0 ) {
            smsc_log_warn(log_,"there were no requests during last %u msec, stopping",
                          unsigned(previous_.elapsedTime));
            stopped_ = true;
            mon_.notifyAll();
        }
    }
}

} // namespace flooder
} // namespace pvss
} // namespace scag2
