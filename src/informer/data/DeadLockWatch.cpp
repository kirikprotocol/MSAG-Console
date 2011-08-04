#include "DeadLockWatch.h"
#include "CommonSettings.h"
#include "informer/io/Typedefs.h"
#include "informer/snmp/SnmpManager.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace informer {

DeadLockWatch::DeadLockWatch( DeadLockWatcher& watcher,
                              const char* category, const char* name ) :
watcher_(watcher),
category_(category),
name_(name),
serial_(1),
prevSerial_(0) 
{
    watcher.registerWatch(*this);
}


DeadLockWatch::~DeadLockWatch()
{
    watcher_.unregisterWatch(*this);
}


DeadLockWatcher::DeadLockWatcher() :
stopping_(false)
{
}


DeadLockWatcher::~DeadLockWatcher()
{
    setStopping();
    WaitFor();
}


void DeadLockWatcher::start()
{
    MutexGuard mg(mon_);
    stopping_ = false;
    Start();
}


void DeadLockWatcher::setStopping()
{
    MutexGuard mg(mon_);
    stopping_ = true;
    mon_.notify();
}


int DeadLockWatcher::Execute()
{
    const msgtime_type interval = 60;
    msgtime_type nextTime = currentTimeSeconds();
    while ( ! stopping_ ) {
        MutexGuard mg(mon_);
        const msgtime_type currentTime = currentTimeSeconds();
        if ( currentTime < nextTime ) {
            // sleeping
            int waitTime = int(nextTime - currentTime)*1000 + 300;
            mon_.wait( waitTime );
            continue;
        }
        if ( stopping_ ) { break; }
        nextTime = currentTime + interval;

        // processing all items
        for ( WatchList::const_iterator i = watches_.begin(), ie = watches_.end();
              i != ie; ++i ) {
            if ( ! (*i)->isAlive() ) {
                // report dead lock
                if ( getCS()->getSnmp() ) {
                    getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_DEADLOCK,
                                                  SnmpTrap::SEV_MAJOR,
                                                  (*i)->getCategory(),
                                                  (*i)->getName(),
                                                  "dead lock detected" );
                }
                fprintf(stderr,"deadlock (%llu) in %s '%s'\n",
                        msgTimeToYmd(currentTime),
                        (*i)->getCategory(), (*i)->getName() );
            }
        }
    }
    return 0;
}


void DeadLockWatcher::registerWatch( DeadLockWatch& watch )
{
    MutexGuard mg(mon_);
    watches_.push_back( &watch );
    mon_.notify();
}


void DeadLockWatcher::unregisterWatch( DeadLockWatch& watch )
{
    MutexGuard mg(mon_);
    WatchList::iterator i = std::find(watches_.begin(),watches_.end(),&watch);
    if ( i != watches_.end() ) {
        watches_.erase(i);
    }
}

}
}
