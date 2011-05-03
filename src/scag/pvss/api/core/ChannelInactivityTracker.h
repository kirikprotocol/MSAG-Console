#ifndef _SCAG_PVSS_CORE_CHANNELINACTIVITYTRACKER_H
#define _SCAG_PVSS_CORE_CHANNELINACTIVITYTRACKER_H

#include <list>
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "ChannelInactivityListener.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/util/Time.h"
#include "scag/util/XHashPtrFunc.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

class ChannelInactivityTracker : public util::WatchedThreadedTask
{
private:
    typedef smsc::core::buffers::XHash<smsc::core::network::Socket*,util::msectime_type,util::XHashPtrFunc>  TimeHash;

public:
    ~ChannelInactivityTracker() {
        shutdown();
    }

    ChannelInactivityTracker( ChannelInactivityListener& theListener, util::msectime_type theInactivityTime ) :
    inactivityTime(theInactivityTime),
    listener(theListener),
    started(false),
    log_(smsc::logger::Logger::getInstance(taskName()))
    {}

    virtual const char* taskName() /*const*/ { return "CITracker"; }

    bool removeChannel( smsc::core::network::Socket& channel )
    {
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        return activityTimes.Delete(&channel);
    }

    void registerChannel( PvssSocket& channel, util::msectime_type tmo )
    {
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        util::msectime_type* ptr = activityTimes.GetPtr(channel.socket());
        if ( ptr ) *ptr = tmo;
        else activityTimes.Insert( channel.socket(), tmo );
    }

    void updateChannel( PvssSocket& channel, util::msectime_type tmo )
    {
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        util::msectime_type* ptr = activityTimes.GetPtr(channel.socket());
        if ( ptr ) *ptr = tmo;
        // else activityTimes.Insert( channel.socket(), tmo );
    }

    void shutdown()
    {
        if (!started) return;
        {
            smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
            started = false;
            activityTimesMutex.notify();
        }
        waitUntilReleased();
    }

    void run()
    {
        util::msectime_type minTimeToSleep = 100; // 100 ms

        util::msectime_type timeToSleep = inactivityTime;
        util::msectime_type nextWakeupTime = util::currentTimeMillis() + timeToSleep;

        smsc_log_info(log_,"starting inactivity time tracker, tmo=%llu ms",timeToSleep);

        started = true;
        while (started)
        {

            std::list< smsc::core::network::Socket* > expiredList;
            {
                smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
                // try {
                activityTimesMutex.wait( int((timeToSleep < minTimeToSleep) ? minTimeToSleep : timeToSleep) );
                // }
                // catch (InterruptedException e) {
                //     e.printStackTrace(); 
                // }

                if (!started) break;

                const util::msectime_type currentTime = util::currentTimeMillis();
                if (nextWakeupTime > currentTime) {
                    timeToSleep = nextWakeupTime - currentTime;
                    continue;
                }

                timeToSleep = inactivityTime;
                smsc::core::network::Socket* sock;
                util::msectime_type*   entry; // pointer to the inactivity time
                for ( TimeHash::Iterator i(&activityTimes); i.Next(sock,entry); ) {
                    const util::msectime_type nextPingTime = *entry + inactivityTime;
                    if (currentTime >= nextPingTime) {
                        // expired
                        smsc_log_debug(log_,"channel %p was last active on %llu, need ping on %llu, cur=%llu, so expired",
                                       sock, *entry, nextPingTime, currentTime );
                        *entry = currentTime;
                        expiredList.push_back( sock );
                        // listener.inactivityTimeoutExpired(*sock);
                    } else {
                        const util::msectime_type sleepTime = nextPingTime - currentTime;
                        if (timeToSleep > sleepTime) {
                            nextWakeupTime = nextPingTime;
                            timeToSleep = sleepTime;
                        }
                    }
                }

            } // guard clause

            while ( ! expiredList.empty() ) {
                smsc::core::network::Socket* sock = expiredList.front();
                expiredList.pop_front();
                listener.inactivityTimeoutExpired(*sock);
            }

        }
        started = false;
    }

protected:
    virtual int doExecute()
    {
        smsc_log_info(log_,"executing %s...", taskName() );
        run();
        smsc_log_info(log_,"%s::Execute() finished()", taskName() );
        return 0;
    }

private:
    util::msectime_type                       inactivityTime;
    ChannelInactivityListener&                listener;
    smsc::core::synchronization::EventMonitor activityTimesMutex;
    TimeHash                                  activityTimes;
    bool                                      started;
    smsc::logger::Logger*                     log_;

};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CHANNELACTIVITYLISTENER_H */
