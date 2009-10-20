#include <algorithm>
#include "Sender.h"
#include "Message.h"
#include "Connector.h"
#include "ProtoException.h"
#include "scag/util/PtrDestroy.h"
#include "scag/util/PtrLess.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

Sender::Sender() :
log_(smsc::logger::Logger::getInstance("sender")),
default_(size_t(-1))
{
    connectors_.reserve(200);
}


Sender::~Sender()
{
    std::for_each( connectors_.begin(), connectors_.end(), PtrDestroy() );
    connectors_.clear();
}


void Sender::addConnector( Connector* conn )
{
    MutexGuard mg(lock_);
    connectors_.push_back(conn);
    std::sort( connectors_.begin(), connectors_.end(), PtrLess() );
    if ( default_ == size_t(-1) ) default_ = 0;
}


unsigned Sender::connectorCount()
{
    MutexGuard mg(lock_);
    return unsigned(connectors_.size());
}


int Sender::send( unsigned deltaTime, Message& msg )
{
    unsigned regid = msg.getRegionId();
    MutexGuard mg(lock_);
    ConnectorList::iterator i = findConnector(regid);
    if ( i == connectors_.end() ) {
        if ( default_ == size_t(-1) ) return MessageState::NOCONN;
        i = connectors_.begin() + default_;
    }
    int reason = (*i)->send( deltaTime, msg );

    // resorting it
    resort(i);
    return reason;
}


void Sender::dumpStatistics( std::string& s )
{
    MutexGuard mg(lock_);
    for ( ConnectorList::const_iterator i = connectors_.begin();
          i != connectors_.end(); ++i ) {
        s.append("\n  ");
        s.append( (*i)->toString());
    }
}


unsigned Sender::hasReadyConnector( unsigned deltaTime, unsigned& regionId )
{
    MutexGuard mg(lock_);
    if ( log_->isDebugEnabled() ) {
        for ( ConnectorList::const_iterator i = connectors_.begin();
              i != connectors_.end();
              ++i ) {
            smsc_log_debug(log_,"%3u %s", unsigned(i-connectors_.begin()), (*i)->toString().c_str() );
        }
    }

    unsigned sleepms = 500;
    for ( ConnectorList::iterator i = connectors_.begin();
          i != connectors_.end();
          ++i ) {
        Connector* conn = *i;
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


void Sender::suspendConnector( unsigned deltaTime, unsigned regionId )
{
    MutexGuard mg(lock_);
    ConnectorList::iterator i = findConnector(regionId);
    if ( i != connectors_.end() ) {
        (*i)->suspend(deltaTime);
        resort(i);
    }
}


Sender::ConnectorList::iterator Sender::findConnector( unsigned regionId )
{
    ConnectorList::iterator res = connectors_.begin();
    for ( ; res != connectors_.end() && (*res)->getId() != regionId; ++res ) {
    }
    return res;
}


void Sender::resort( ConnectorList::iterator i )
{
    Connector* c = *i;
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

}
}
}
