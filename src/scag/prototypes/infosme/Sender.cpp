#include <cassert>
#include <algorithm>
#include "Sender.h"
#include "Message.h"
#include "Connector.h"
#include "ProtoException.h"
#include "TaskDispatcher.h"
#include "util/PtrDestroy.h"
#include "util/PtrLess.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

Sender::Sender( TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance("sender")),
scoredList_(*this,5000,log_),
default_(0),
dispatcher_(&disp)
{
}


Sender::~Sender()
{
    MutexGuard mg(lock_);
    for ( size_t i = 0; i < scoredList_.size(); ++i ) {
        Region* c = scoredList_[i];
        delete c;
    }
    scoredList_.clear();
}


void Sender::addRegion( Region* conn )
{
    if ( ! conn ) return;
    MutexGuard mg(lock_);
    scoredList_.add(conn);
    dispatcher_->addRegion(*conn);
    if ( default_ == 0 ) default_ = conn;
}


unsigned Sender::connectorCount()
{
    MutexGuard mg(lock_);
    return unsigned(scoredList_.size());
}

/*
int Sender::send( unsigned deltaTime, Message& msg )
{
    unsigned regid = msg.getRegionId();
    MutexGuard mg(lock_);
    RegionList::iterator i = findRegion(regid);
    if ( i == connectors_.end() ) {
        if ( default_ == size_t(-1) ) return MessageState::NOCONN;
        i = connectors_.begin() + default_;
    }
    int reason = (*i)->send( deltaTime, msg );

    // resorting it
    resort(i);
    return reason;
}
 */


void Sender::dumpStatistics( std::string& s )
{
    MutexGuard mg(lock_);
    scoredList_.dump(s);
}


/*
unsigned Sender::hasReadyRegion( unsigned deltaTime, unsigned& regionId )
{
    MutexGuard mg(lock_);
    if ( log_->isDebugEnabled() ) {
        for ( RegionList::const_iterator i = connectors_.begin();
              i != connectors_.end();
              ++i ) {
            smsc_log_debug(log_,"%3u %s", unsigned(i-connectors_.begin()), (*i)->toString().c_str() );
        }
    }

    unsigned sleepms = 500;
    for ( RegionList::iterator i = connectors_.begin();
          i != connectors_.end();
          ++i ) {
        Region* conn = *i;
        unsigned wantToSleep = conn->isReady(deltaTime);
        if ( wantToSleep == 0 ) {
            regionId = conn->getId();
            sleepms = 0;
            break;
        }
        if ( wantToSleep < sleepms ) sleepms = wantToSleep;
    }
    return sleepms;
}


void Sender::suspendRegion( unsigned deltaTime, unsigned regionId )
{
    MutexGuard mg(lock_);
    RegionList::iterator i = findRegion(regionId);
    if ( i != connectors_.end() ) {
        (*i)->suspend(deltaTime);
        resort(i);
    }
}
 */


unsigned Sender::send( unsigned deltaTime, unsigned sleepTime )
{
    std::string s;
    MutexGuard mg(lock_);
    if ( log_->isDebugEnabled() ) {
        s.reserve(400);
        scoredList_.dump(s);
        smsc_log_debug(log_,"regions (%u) are:%s",unsigned(scoredList_.size()),s.c_str());
    }
    return scoredList_.processOnce(deltaTime,sleepTime);
}


/*
Sender::RegionList::iterator Sender::findRegion( unsigned regionId )
{
    RegionList::iterator res = connectors_.begin();
    for ( ; res != connectors_.end() && (*res)->getId() != regionId; ++res ) {
    }
    return res;
}


void Sender::resort( RegionList::iterator i )
{
    Region* c = *i;
    const size_t oldpos = (i - connectors_.begin());
    connectors_.erase(i);
    i = std::lower_bound( connectors_.begin(), connectors_.end(), c, PtrLess() );
    connectors_.insert(i,c);
    const size_t newpos = (i - connectors_.begin());
    if ( oldpos == default_ ) {
        default_ = newpos;
    } else {
        if ( oldpos <= default_ ) --default_;
        if ( newpos <= default_ ) ++default_;
    }
}
 */


void Sender::scoredObjToString( std::string& s, Region& c )
{
    s.append(c.toString());
}


unsigned Sender::scoredObjIsReady( unsigned deltaTime, Region& c )
{
    return c.isReady( deltaTime );
}


int Sender::processScoredObj( unsigned deltaTime, Region& c )
{
    unsigned inc = 1000/c.getBandwidth();
    try {
        unsigned wantToSleep = dispatcher_->processRegion( deltaTime, c );
        smsc_log_debug(log_,"connector %u processed, res=%u",c.getId(),wantToSleep);
        if ( wantToSleep > 0 ) {
            // all tasks want to sleep
            c.suspend(deltaTime+wantToSleep);
            return -inc;
        }
    } catch ( std::exception& exc ) {
        smsc_log_debug(log_,"sending failed: %s", exc.what() );
        c.suspend(deltaTime+1000);
        return -1000;
    }
    return inc;
}

}
}
}
