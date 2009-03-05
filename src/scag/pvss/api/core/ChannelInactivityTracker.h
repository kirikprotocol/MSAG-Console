#ifndef _SCAG_PVSS_CORE_CHANNELINACTIVITYTRACKER_H
#define _SCAG_PVSS_CORE_CHANNELINACTIVITYTRACKER_H

#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "ChannelInactivityListener.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/util/Time.h"
#include "scag/util/XHashPtrFunc.h"

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocket;

class ChannelInactivityTracker : public util::WatchedThreadedTask
{
private:
    typedef smsc::core::buffers::XHash<PvssSocket*,long,util::XHashPtrFunc>  TimeHash;

public:
    ~ChannelInactivityTracker() {
        shutdown();
    }

    ChannelInactivityTracker( ChannelInactivityListener& theListener, long theInactivityTime ) :
    inactivityTime(theInactivityTime),
    listener(theListener),
    started(false)
    {}

    virtual const char* taskName() /*const*/ { return "CITracker"; }

    virtual int Execute()
    {
        run();
        return 0;
    }

    bool removeChannel( PvssSocket& channel )
    {
        MutexGuard mg(activityTimesMutex);
        return activityTimes.Delete(&channel);
    }
    void registerChannel( PvssSocket& channel, long tmo )
    {
        MutexGuard mg(activityTimesMutex);
        long* ptr = activityTimes.GetPtr(&channel);
        if ( ptr ) *ptr = tmo;
        else activityTimes.Insert( &channel, tmo );
    }
    void shutdown()
    {
        if (!started) return;
        {
            MutexGuard mg(activityTimesMutex);
            started = false;
            activityTimesMutex.notify();
        }
        waitUntilReleased();
    }

    void run()
    {
        long minTimeToSleep = 100; // 100 ms

        long timeToSleep = inactivityTime;
        long nextWakeupTime = util::currentTimeMillis() + timeToSleep;

        started = true;
        while (started)
        {
            MutexGuard mg(activityTimesMutex);
            // try {
            activityTimesMutex.wait( (timeToSleep < minTimeToSleep) ? minTimeToSleep : timeToSleep ); 
            // }
            // catch (InterruptedException e) {
            //     e.printStackTrace(); 
            // }

            if (!started) break;

            const long currentTime = util::currentTimeMillis();
            if (nextWakeupTime > currentTime) {
                timeToSleep = nextWakeupTime - currentTime;
                continue;
            }

            timeToSleep = inactivityTime;
            PvssSocket* sock;
            long*   entry;
            for ( TimeHash::Iterator i(&activityTimes); i.Next(sock,entry); ) {
                long nextPingTime = *entry + inactivityTime;
                if (currentTime >= nextPingTime) {
                    *entry = util::currentTimeMillis();
                    listener.inactivityTimeoutExpired(*sock);
                }
                else {
                    long sleepTime = nextPingTime - currentTime;
                    if (timeToSleep > sleepTime) {
                        nextWakeupTime = nextPingTime;
                        timeToSleep = sleepTime;
                    }
                }
            }
        }
        started = false;
    }

private:
    long                                      inactivityTime;
    ChannelInactivityListener&                listener;
    smsc::core::synchronization::EventMonitor activityTimesMutex;
    TimeHash                                  activityTimes;
    bool                                      started;

};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CHANNELACTIVITYLISTENER_H */
