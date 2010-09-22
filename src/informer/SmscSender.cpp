#include "SmscSender.h"
#include "InfosmeException.h"
#include "InfosmeCore.h"
#include "Typedefs.h"
#include "RegionSender.h"

namespace smsc {
namespace informer {

SmscSender::SmscSender( InfosmeCore& core,
                        const std::string& smscId,
                        const smsc::sme::SmeConfig& cfg ) :
log_(0), core_(&core),
smscId_(smscId), session_(0),
scoredList_(0)
{
    log_ = smsc::logger::Logger::getInstance("snd");
    session_.reset( new smsc::sme::SmppSession(cfg,this) );
    scoredList_ = new ScoredList< SmscSender >(*this,500000,0);
}


SmscSender::~SmscSender()
{
    if (session_.get()) session_->close();
    delete scoredList_;
}


unsigned SmscSender::send( msgtime_type currentTime,
                           Delivery& dlv,
                           regionid_type regionId,
                           Message& msg )
{
    smsc_log_error(log_,"FIXME: send()");
    return 0;
}


void SmscSender::updateConfig( const smsc::sme::SmeConfig& config )
{
    waitUntilReleased();
    // FIXME: recreate session
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
            // FIXME: create a new session from config
        } else if ( !session_->isClosed() ) {
            break;
        }
        // FIXME: connect, then sleep if failed
    }
}


void SmscSender::sendLoop()
{
    const unsigned sleepTime = 5000000; // 5 sec

    usectime_type currentTime = currentTimeMicro();
    usectime_type movingStart = currentTime;
    usectime_type nextWakeTime = currentTime;
    while ( !isStopping ) {

        MutexGuard mg(mon_);
        if ( !session_.get() || session_->isClosed() ) break;

        // 1. sleeping until next wake time
        currentTime = currentTimeMicro();
        int waitTime = int((nextWakeTime - currentTime)/1000); // in msec

        if (waitTime > 0) {
            if (waitTime < 10) waitTime = 10;
            mon_.wait(waitTime);
        }

        // 2. flipping start time
        unsigned deltaTime = unsigned(currentTime - movingStart);
        if (deltaTime > 1000000000U ) {
            movingStart += deltaTime;
            smsc_log_debug(log_,"moving start time +%u",deltaTime);
            deltaTime = 0;
        }

        nextWakeTime = currentTime + scoredList_->processOnce( deltaTime, sleepTime );
        processWaitingEvents(currentTime);
    }
    if (session_.get() && !session_->isClosed()) session_->close();
}


unsigned SmscSender::scoredObjIsReady( unsigned currentTime, ScoredObjType& regionSender )
{
    smsc_log_error(log_,"FIXME: scored obj is ready at %u", unsigned(currentTime));
    return 10000;
}


int SmscSender::processScoredObj( unsigned currentTime, ScoredObjType& regionSender )
{
    smsc_log_error(log_,"FIXME: process scored obj at %u", unsigned(currentTime));
    return -10000;
}


void SmscSender::scoredObjToString( std::string& s, ScoredObjType& regionSender )
{
    s += regionSender.toString();
}


void SmscSender::processWaitingEvents( msgtime_type currentTime )
{
    smsc_log_error(log_,"FIXME: process waiting events at %u", unsigned(currentTime));
}


}
}
