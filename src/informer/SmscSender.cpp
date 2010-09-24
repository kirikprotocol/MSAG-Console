#include "SmscSender.h"
#include "InfosmeException.h"
#include "InfosmeCore.h"
#include "Typedefs.h"
#include "RegionSender.h"

namespace eyeline {
namespace informer {

SmscSender::SmscSender( InfosmeCore& core,
                        const std::string& smscId,
                        const smsc::sme::SmeConfig& cfg ) :
log_(smsc::logger::Logger::getInstance("smscsend")),
core_(&core),
smscId_(smscId), session_(0),
scoredList_(*this,2*maxScoreIncrement,log_)
{
    session_.reset( new smsc::sme::SmppSession(cfg,this) );
}


SmscSender::~SmscSender()
{
    if (session_.get()) session_->close();
}


unsigned SmscSender::send( RegionalStoragePtr& ptr,
                           Message& msg )
{
    smsc_log_error(log_,"FIXME: send()");
    return 0;
}


void SmscSender::updateConfig( const smsc::sme::SmeConfig& config )
{
    waitUntilReleased();
    MutexGuard mg(mon_);
    session_.reset(new smsc::sme::SmppSession(config,this));
}


void SmscSender::waitUntilReleased()
{
    if (isReleased) return;
    stop();
    while (true) {
        MutexGuard mg(mon_);
        if (isReleased) break;
        mon_.wait(100);
    }
}


void SmscSender::handleEvent( smsc::sme::SmppHeader* pdu )
{
    smsc_log_error(log_,"FIXME: handleEvent, S=%s", smscId_.c_str());
}


void SmscSender::handleError( int errorcode )
{
    smsc_log_error(log_,"FIXME: handleError, S=%s", smscId_.c_str());
}


int SmscSender::Execute()
{
    while ( !isStopping ) {
        connectLoop();
        if (isStopping) break;
        sendLoop();
    }
    return 0;
}


void SmscSender::onThreadPoolStartTask()
{
    isStopping = false;
    isReleased = false;
}


void SmscSender::onRelease()
{
    MutexGuard mg(mon_);
    isReleased = true;
    mon_.notify();
}


void SmscSender::connectLoop()
{
    while ( !isStopping ) {
        MutexGuard mg(mon_);
        if ( !session_.get() ) {
            smsc_log_error(log_,"FIXME: session is not configured");
            isStopping = true;
        } else if ( !session_->isClosed() ) {
            // session connected
            break;
        }
        smsc_log_error(log_,"FIXME: connect session");
        mon_.wait(1000); // FIXME: configure timeout
    }
}


void SmscSender::sendLoop()
{
    const unsigned sleepTime = 5000000U; // 5 sec

    currentTime_ = currentTimeMicro();
    // usectime_type movingStart = currentTime_;
    usectime_type nextWakeTime = currentTime_;
    while ( !isStopping ) {

        MutexGuard mg(mon_);
        if ( !session_.get() || session_->isClosed() ) break;

        // 1. sleeping until next wake time
        currentTime_ = currentTimeMicro();
        int waitTime = int((nextWakeTime - currentTime_)/1000U); // in msec

        if (waitTime > 0) {
            if (waitTime < 10) waitTime = 10;
            mon_.wait(waitTime);
        }

        nextWakeTime = currentTime_ + scoredList_.processOnce(0, sleepTime);
        processWaitingEvents();
    }
    if (session_.get() && !session_->isClosed()) session_->close();
}


unsigned SmscSender::scoredObjIsReady( unsigned, ScoredObjType& regionSender )
{
    const unsigned ret = regionSender.isReady(currentTime_);
    smsc_log_debug(log_,"R=%u waits %u usec until ready()", regionSender.getRegionId(), ret);
    return ret;
}


int SmscSender::processScoredObj( unsigned, ScoredObjType& regionSender )
{
    unsigned inc = maxScoreIncrement/regionSender.getBandwidth();
    try {
        const unsigned wantToSleep = regionSender.processRegion(currentTime_);
        smsc_log_debug(log_,"R=%u processed, sleep=%u", regionSender.getRegionId(), wantToSleep);
        if (wantToSleep>0) {
            // all deliveries want to sleep
            regionSender.suspend(currentTime_ + wantToSleep);
            return -inc;
        }
    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"R=%u send exc: %s", regionSender.getRegionId(), e.what());
        regionSender.suspend(currentTime_ + 1000000U);
    }
    return inc;
}


void SmscSender::scoredObjToString( std::string& s, ScoredObjType& regionSender )
{
    s += regionSender.toString();
}


void SmscSender::processWaitingEvents()
{
    smsc_log_error(log_,"FIXME: process waiting events at %llu", currentTime_);
}


}
}
