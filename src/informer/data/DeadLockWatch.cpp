#include "DeadLockWatch.h"
#include "CommonSettings.h"
#include "informer/snmp/SnmpManager.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace informer {

DeadLockWatch::DeadLockWatch( DeadLockWatcher& watcher,
                              const char* category,
                              const char* name,
                              timediff_type period ) :
watcher_(watcher),
category_(category),
name_(name),
period_(period),
lastTime_(currentTimeSeconds())
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
        nextTime = currentTime + getCS()->getDeadLockWatchPeriod();

        // processing all items
        for ( WatchList::const_iterator i = watches_.begin(), ie = watches_.end();
              i != ie; ++i ) {
            msgtime_type dead = (*i)->isAlive( currentTime );
            if ( dead > 0 ) {
                // report dead lock
                if ( getCS()->getSnmp() ) {
                    char buf[80];
                    sprintf(buf,"deadlock/threadexit detected during %u seconds",unsigned(dead));
                    getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_DEADLOCK,
                                                  SnmpTrap::SEV_MAJOR,
                                                  (*i)->getCategory(),
                                                  (*i)->getName(),
                                                  buf );
                }
                fprintf(stderr,"%llu deadlock in %s '%s' for %u seconds\n",
                        msgTimeToYmd(currentTime),
                        (*i)->getCategory(), (*i)->getName(),
                        unsigned(dead));
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
