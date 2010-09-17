#ifndef _INFOSME_V3_SMSCCONNECTOR_H
#define _INFOSME_V3_SMSCCONNECTOR_H

#include <list>
#include <map>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "Typedefs.h"
#include "ReceiptReceiver.h"
#include "Delivery.h"

namespace smsc {
namespace infosme {

class SmscConnector : protected smsc::core::threads::Thread
{
private:
    typedef std::list< DlvRegMsgId > ResponseList;
    typedef std::multimap< msgtime_type, ResponseList::iterator > ResponseMap;

public:
    SmscConnector( ReceiptReceiver& rr ) :
    rr_(&rr),
    log_(smsc::logger::Logger::getInstance("smsconn")),
    stopping_(true) {}

    virtual ~SmscConnector() {
        stop();
        WaitFor();
    }

    /// send msg and returns the number of chunks sent or 0.
    int send( msgtime_type currentTime, Delivery& dlv, regionid_type regionId, Message& msg );

    
    void start() {
        if (!stopping_) return;
        MutexGuard mg(mon_);
        if (!stopping_) return;
        stopping_ = false;
        Start();
    }

    void stop() {
        if (stopping_) return;
        MutexGuard mg(mon_);
        if (stopping_) return;
        stopping_ = true;
        mon_.notifyAll();
    }

protected:

    void scheduleResponse( msgtime_type respTime, const DlvRegMsgId& dlvmsg )
    {
        MutexGuard mg(mon_);
        ResponseMap::iterator iter = responseMap_.upper_bound(respTime);
        ResponseList::iterator j = responseList_.insert( iter == responseMap_.end() ?
                                                         responseList_.end() :
                                                         iter->second,
                                                         dlvmsg );
        responseMap_.insert( iter, std::make_pair(respTime,j) );
        if ( log_->isDebugEnabled() ) {
            const msgtime_type now = msgtime_type(time(0));
            smsc_log_debug(log_,"response for R=%u/D=%u/M=%u scheduled for %+d",
                           unsigned(dlvmsg.regId), unsigned(dlvmsg.dlvId),
                           unsigned(dlvmsg.msgId), int(respTime - now) );
        }
        mon_.notifyAll();
    }


    virtual int Execute()
    {
        smsc_log_debug(log_,"started");
        while (!stopping_) {

            const msgtime_type now = msgtime_type(time(0));
            ResponseList workingList;
            {
                MutexGuard mg(mon_);
                ResponseMap::iterator iter = responseMap_.upper_bound(now);
                if ( iter == responseMap_.begin() ) {
                    // no elements at this time
                    const timediff_type diff = 
                        ( responseMap_.empty() ? 5 :
                          timediff_type(responseMap_.begin()->first - now));
                    smsc_log_debug(log_,"next response will be in %d seconds",diff);
                    if ( diff > 0 ) {
                        mon_.wait( diff*1000 );
                    }
                    continue;
                }

                // moving working elements to the working list
                workingList.splice(workingList.begin(),responseList_,
                                   responseList_.begin(),
                                   iter == responseMap_.end() ?
                                   responseList_.end() :
                                   iter->second );
                responseMap_.erase(responseMap_.begin(), iter);
            }

            // processing working list
            for ( ResponseList::iterator i = workingList.begin();
                  i != workingList.end(); ++i ) {
                try {
                    rr_->receiptReceived( now, *i, MsgState::delivered, 0 );
                    smsc_log_debug(log_,"receipt %u/%u processed",
                                   i->dlvId, i->msgId );
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"receipt %u/%u cannot be processed",
                                  i->dlvId, i->msgId );
                }
            }

        }
        smsc_log_debug(log_,"finished");
        return 0;
    }

protected:
    smsc::core::synchronization::EventMonitor mon_;
    ResponseList          responseList_;
    ResponseMap           responseMap_;
    ReceiptReceiver*      rr_;
    smsc::logger::Logger* log_;
    bool                  stopping_;
};

} // infosme
} // smsc

#endif
