#ifndef _EYELINE_SMPP_RESPREGISTRY_H
#define _EYELINE_SMPP_RESPREGISTRY_H

#include <list>
#include "core/threads/Thread.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "eyeline/smpp/Utility.h"
#include "eyeline/smpp/pdu/Pdu.h"

namespace eyeline {
namespace smpp {

struct ExpiredPduListener
{
    virtual ~ExpiredPduListener() {}

    /// NOTE: ownership is passed
    virtual void pduExpired( Pdu* pdu ) = 0;
    /// NOTE: ownership is passed
    virtual void socketClosed( unsigned sockid, Pdu* pdu ) = 0;
};


class RespRegistry : public smsc::core::threads::Thread
{
    struct PduItem 
    {
        Pdu*          pdu;
        msectime_type expire;
        unsigned      sockid; // socketid or 0
    };

    typedef std::list< PduItem > PduList;
    typedef smsc::core::buffers::IntHash< PduList::iterator > PduHash;

public:
    RespRegistry( ExpiredPduListener& listener,
                  unsigned expireTimeout ) :
    log_(smsc::logger::Logger::getInstance("smpp.reg")),
    listener_(listener),
    expireTmo_(expireTimeout),
    stopping_(false) {}
        

    ~RespRegistry()
    {
    }


    void registerPdu( Pdu* pdu )
    {
        if (!pdu) return;
        std::auto_ptr< Pdu > pduptr(pdu);
        const msectime_type now = currentTimeMillis();
        smsc::core::synchronization::MutexGuard mg(mon_);
        if (stopping_) {
            throw SmppException("core is stopping");
        }
        PduList::iterator* iter = pduHash_.GetPtr(pdu->getSeqNum());
        const bool wake = pduList_.empty();
        PduList::iterator i;
        if (iter) {
            char oldb[200], newb[200];
            smsc_log_warn(log_,"pdu seq=%u is replaced: old=%s new=%s",
                          pdu->getSeqNum(),
                          (*iter)->pdu->toString(oldb,sizeof(oldb)),
                          pdu->toString(newb,sizeof(newb)));
            i = *iter;
            delete i->pdu;
            pduList_.splice(pduList_.end(),pduList_,i);
        } else {
            i = pduList_.insert(pduList_.end(),PduItem());
            pduHash_.Insert(pdu->getSeqNum(),i);
        }
        i->pdu = pdu;
        i->expire = now + expireTmo_;
        i->sockid = 0;
        pduptr.release();
        // registering
        smsc_log_debug(log_,"pdu seq=%u registered",pdu->getSeqNum());
        if (wake) { mon_.notify(); }
    }


    void setSocketId( uint32_t seqNum, unsigned socketId )
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        PduList::iterator* i = pduHash_.GetPtr(seqNum);
        if (!i) return;
        (*i)->sockid = socketId;
    }


    Pdu* unregisterPdu( uint32_t seqNum )
    {
        Pdu* res = 0;
        smsc::core::synchronization::MutexGuard mg(mon_);
        PduList::iterator iter;
        if ( pduHash_.Pop(seqNum,iter) ) {
            res = iter->pdu;
            pduList_.erase(iter);
        }
        return res;
    }
    

    void socketClosed( unsigned sockid )
    {
        PduList working;
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            for ( PduList::iterator i = pduList_.begin(), ie = pduList_.end();
                  i != ie; ) {
                PduList::iterator j = i++;
                if ( j->sockid == sockid ) {
                    working.splice(working.end(),pduList_,j);
                }
            }
            for ( PduList::iterator i = working.begin(), ie = working.end();
                  i != ie; ++i ) {
                pduHash_.Delete(i->pdu->getSeqNum());
            }
        }
        for ( PduList::iterator i = working.begin(), ie = working.end();
              i != ie; ++i ) {
            listener_.socketClosed(sockid,i->pdu);
        }
    }


    virtual int Execute()
    {
        PduList working;
        smsc_log_info(log_,"started");
        while (true) {

            {
                smsc::core::synchronization::MutexGuard mg(mon_);
                const msectime_type now = currentTimeMillis();
                if (stopping_) {
                    break;
                }

                int delta = 5000;
                if (!pduList_.empty()) {
                    delta = int(pduList_.front().expire - now);
                }
                if ( delta > 0 ) {
                    if ( delta > 5000 ) { delta = 5000; }
                    mon_.wait(delta);
                    continue;
                }

                // extract things from the list
                PduList::iterator i = pduList_.begin();
                for ( ; i != pduList_.end(); ++i ) {
                    if ( i->expire > now ) break;
                    pduHash_.Delete(i->pdu->getSeqNum());
                }
                working.splice(working.begin(),
                               pduList_,
                               pduList_.begin(),
                               i);
            }

            // and now process working
            for ( PduList::iterator i = working.begin(), ie = working.end();
                  i != ie; ++i ) {
                char buf[200];
                smsc_log_debug(log_,"pdu expired %s",i->pdu->toString(buf,sizeof(buf)));
                listener_.pduExpired(i->pdu);
            }
            working.clear();
        }

        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            pduHash_.Empty();
            working.splice(working.begin(),pduList_);
        }
        for ( PduList::iterator i = working.begin(), ie = working.end();
              i != ie; ++i ) {
            if ( i->sockid ) {
                listener_.socketClosed(i->sockid,i->pdu);
            }
        }
        smsc_log_info(log_,"finished");
        return 0;
    }


    void start()
    {
        Start();
    }


    void stop()
    {
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            stopping_ = true;
            mon_.notify();
        }
        WaitFor();
    }

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::EventMonitor mon_;
    PduList               pduList_;
    PduHash               pduHash_;
    ExpiredPduListener&   listener_;
    unsigned              expireTmo_;
    bool                  stopping_;
};

}
}

#endif
