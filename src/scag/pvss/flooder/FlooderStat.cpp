#include "FlooderStat.h"
#include "FlooderThread.h"
#include "scag/util/HRTimer.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/util/histo/HistoLogger.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace pvss {
namespace flooder {

const double FlooderStat::binScale = 1.5;

FlooderStat::FlooderStat( const FlooderConfig& config,
                          core::client::Client& client ) :
log_(0),
requestsPerSecond_(config.getSpeed()),
requested_(config.getRequested()),
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
lastHistoCreateTime_(util::currentTimeMillis()),
outReady_(false),
outTotal_(0),
outLast_(accumulationTime),
histoReady_(false),
outTotalHisto_(binScale,histoBins,minTime),
outLastHisto_(binScale,histoBins,minTime),
outHistoCreateTime_(0),
decreaseCount_(0),
maxSpeed_(0)
{
    log_ = smsc::logger::Logger::getInstance("flooder");
}

FlooderStat::~FlooderStat()
{
  if (maxSpeed_ > 0) {
    smsc_log_info( log_, "max speed: %d", maxSpeed_);
  }

    shutdown(); 
}


void FlooderStat::handleResponse( std::auto_ptr< Request > request, std::auto_ptr< Response > response )
{
    smsc_log_debug(log_,"response got: req=%s resp=%s", request->toString().c_str(), response->toString().c_str() );
    // checking timer
    unsigned processTime = 0; // in usec
    if ( request.get() ) {
        ProfileRequest* req = static_cast<ProfileRequest*>(request.get());
        const Packet::Timing* timing = req->getTiming();
        if ( timing ) { processTime = timing->total; }
    }
    const bool ok = StatusType::statusIsInfo(response->getStatus());
    {
        MutexGuard mg(mon_);
        ++total_.responses;
        ++last_.responses;
        if ( ok ) {
            ++total_.successes;
            ++last_.successes;
        } else {
            ++total_.failures;
            ++last_.failures;
        }
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
        ProfileRequest* req = static_cast<ProfileRequest*>(request.get());
        req->timingMark("handleErr");
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
    if ( ++counter % requestsPerSecond_ == 0 ) {
        // we would like to have one timing request per second
        static_cast< ProfileRequest* >(req)->startTiming();
        // randomization
        counter += unsigned(reinterpret_cast<uint64_t>(static_cast<const void*>(req))) % 13;
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
    bool needPrinting = false;
    util::msectime_type doneTime = 0;
    util::msectime_type currentTime;
    int waitTime = 1000;

    while ( true ) {

        if ( stopped_ ) break;

        {
            MutexGuard mg(mon_);
            if ( needPrinting ) {
                // we were printing in the previous pass, reset printing flag
                outReady_ = false;
            }
            mon_.wait(1000);
            needPrinting = outReady_;
        }

        if ( needPrinting ) {
            if ( histoReady_ ) {
                currentTime = util::currentTimeMillis();
                scag_plog_info(si,log_);
                util::histo::HistoLogger hl(si);
                hl.printHisto(outTotalHisto_, "total");
                hl.printHisto(outLastHisto_, "last %u sec", unsigned((currentTime - outHistoCreateTime_)/1000) );
                histoReady_ = false;
            }
            std::string fullstat;
            fullstat.reserve(512);
            char buf[100];
            snprintf(buf,sizeof(buf),"Statistics follows: commands=%s", config_.getCommands().c_str() );
            fullstat.append(buf);
            fullstat.append("\n== Total: ");
            fullstat.append( outTotal_.toString() );
            fullstat.append("\n    Last: ");
            fullstat.append( outLast_.toString() );
            smsc_log_info( log_, "%s", fullstat.c_str());

            changeSpeed();

        }


        if ( ! doneTime ) {
            if ( requested_ == 0 || total_.requests < requested_ ) {
                continue;
            }
        }

        // all requests are collected
        currentTime = util::currentTimeMillis();
        if ( ! doneTime ) { doneTime = currentTime; }
        if ( total_.responses + total_.errors >= total_.requests ) {
            break;
        }

        if ( doneTime + timeout_ <= currentTime ) {
            break;
        }
        waitTime = doneTime + timeout_ - currentTime;

    } // while
}

void FlooderStat::changeSpeed() {
  //set max possible speed
  if (!config_.getMaxSpeed() || total_.requests <= last_.requests || outLast_.elapsedTime < 1000) {
    return;
  }
  unsigned maxChange = 50;
  MutexGuard mg(mon_);
  unsigned elapsedTime = static_cast<unsigned>(outLast_.elapsedTime / 1000); 
  unsigned requests = outLast_.requests / elapsedTime;
  unsigned response = outLast_.responses / elapsedTime;
  if (requests > response) {
    unsigned dec = requests - response < maxChange ? requests - response : maxChange;
    requestsPerSecond_ = requestsPerSecond_ - maxChange > maxChange ? requestsPerSecond_ - dec : requestsPerSecond_;
    decreaseCount_ += 1 + dec / 10;
    smsc_log_info( log_, "current speed: %d (-%d)", requestsPerSecond_, dec);
  } else {
    if (requests - maxChange <= requestsPerSecond_ && decreaseCount_ == 0) {
      maxSpeed_ = requestsPerSecond_ > maxSpeed_ ? requestsPerSecond_ : maxSpeed_;
      requestsPerSecond_ += 10;
      smsc_log_info( log_, "current speed: %d (+10)", requestsPerSecond_);
    }
    decreaseCount_ = decreaseCount_ > 0 ? decreaseCount_ - 1 : 0;
  }
}

void FlooderStat::init( unsigned skip ) /* throw (exceptions::IOException) */ 
{
    const std::vector< std::string >& patterns = config_.getPropertyPatterns();
    if ( patterns.size() <= 0 ) throw exceptions::IOException("too few property patterns configured");

    for ( std::vector< std::string >::const_iterator i = patterns.begin();
          i != patterns.end(); ++i ) {
        std::auto_ptr<Property> p( new Property );
        p->fromString( *i );
        generator_.addPropertyPattern( unsigned(i - patterns.begin()), p.release() );
    }
    generator_.parseCommandPatterns( config_.getCommands() );
    if ( log_->isInfoEnabled() ) {
        for ( std::vector< ProfileCommand* >::const_iterator i = generator_.getPatterns().begin();
              i != generator_.getPatterns().end();
              ++i ) {
            smsc_log_info(log_,"pattern #%u: %s", i - generator_.getPatterns().begin(), (*i)->toString().c_str());
        }
    }

    smsc_log_info(log_,"shuffling %u addresses", unsigned(config_.getAddressesCount()));
    generator_.randomizeProfileKeys( config_.getAddressFormat().c_str(),
                                     config_.getAddressesCount(), skip,
                                     config_.getScopeType(),
                                     config_.getAddressOffset() );
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
    if ( last_.checkTime( currentTime ) && !outReady_ ) {
        // a new statistics chunk is filled
        outReady_ = true;
        outTotal_ = total_;
        outLast_ = last_;
        last_.reset();
        if ( lastHisto_.getTotal() >= 300 ) { // 5 min
            histoReady_ = true;
            outTotalHisto_ = totalHisto_;
            outLastHisto_ = lastHisto_;
            outHistoCreateTime_ = lastHistoCreateTime_;
            lastHistoCreateTime_ = currentTime;
            lastHisto_.reset();
        }

        if ( outLast_.requests == 0 ) {
            smsc_log_warn(log_,"there were no requests during last %u msec, stopping",
                          unsigned(outLast_.elapsedTime));
            stopped_ = true;
        }
        mon_.notify();
    }
}
/*
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

    }
}
 */

} // namespace flooder
} // namespace pvss
} // namespace scag2
