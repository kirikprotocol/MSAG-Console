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
#include "core/buffers/Array.hpp"

namespace scag2 {
namespace pvss {
namespace core {

class ChannelInactivityTracker : public util::WatchedThreadedTask
{
    struct SockAndTime
    {
        PvssSockPtr       ptr;
        util::msectime_type tmo;
    };
private:
    typedef smsc::core::buffers::XHash< PvssSocketBase* , SockAndTime, util::XHashPtrFunc >  TimeHash;

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

    bool removeChannel( PvssSocketBase& channel )
    {
//        smsc_log_debug(log_,"removing channel %p sock=%p from inactracker",
//                       &channel, channel.getSocket());
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        return activityTimes.Delete(&channel);
    }

    void registerChannel( PvssSocketBase& channel, util::msectime_type tmo )
    {
        PvssSockPtr temp(&channel);
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        SockAndTime* ptr = activityTimes.GetPtr(&channel);
        if ( ptr ) {
            smsc_log_warn(log_,"re-register a channel %p sock=%p, oldch=%p",
                          &channel, channel.getSocket(), ptr->ptr.get());
            ptr->ptr = temp;
            ptr->tmo = tmo;
        } else {
            SockAndTime& sat = activityTimes.InsertEx( &channel, SockAndTime() );
            sat.ptr = temp;
            sat.tmo = tmo;
        }
    }

    void updateChannel( PvssSocketBase& channel, util::msectime_type tmo )
    {
        smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
        SockAndTime* ptr = activityTimes.GetPtr(&channel);
        if ( ptr ) {
            ptr->tmo = tmo;
        }
        // else activityTimes.Insert( channel.socket(), tmo );
    }

    void shutdown()
    {
        if (!started) return;
        {
            smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
            if (!started) return;
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

        {
            smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
            started = true;
        }
        smsc::core::buffers::Array< PvssSockPtr > expiredList;
        while (started)
        {
            expiredList.Empty();
            {
                smsc::core::synchronization::MutexGuard mg(activityTimesMutex);
                // try {
                activityTimesMutex.wait( int((timeToSleep < minTimeToSleep) ? minTimeToSleep : timeToSleep) );
                // }
                // catch (InterruptedException& e) {
                //     e.printStackTrace(); 
                // }

                if (!started) break;

                const util::msectime_type currentTime = util::currentTimeMillis();
                if (nextWakeupTime > currentTime) {
                    timeToSleep = nextWakeupTime - currentTime;
                    continue;
                }

                timeToSleep = inactivityTime;
                PvssSocketBase* sock;
                SockAndTime*  entry; // pointer to the inactivity time
                for ( TimeHash::Iterator i(&activityTimes); i.Next(sock,entry); ) {
                    const util::msectime_type nextPingTime = entry->tmo + inactivityTime;
                    if (currentTime >= nextPingTime) {
                        // expired
                        smsc_log_debug(log_,"channel %p was last active on %llu, need ping on %llu, cur=%llu, so expired",
                                       sock, entry->tmo, nextPingTime, currentTime );
                        entry->tmo = currentTime;
                        expiredList.Push( entry->ptr );
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

            while ( expiredList.Count() ) {
                PvssSockPtr sock;
                expiredList.Pop(sock);
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
