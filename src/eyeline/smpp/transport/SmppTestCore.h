#ifndef _EYELINE_SMPP_SMPPTESTCORE_H
#define _EYELINE_SMPP_SMPPTESTCORE_H

#include "RespRegistry.h"
#include "Socket.h"
#include "eyeline/smpp/PduListener.h"
#include "eyeline/smpp/SendSubscriber.h"
#include "eyeline/smpp/pdu/ResponsePdu.h"
#include "eyeline/smpp/pdu/PduInputBuf.h"
#include "core/synchronization/AtomicCounter.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace eyeline {
namespace smpp {

/// This class is for testing purposes only.
/// see client/testClient server/testServer.
class SmppTestCore : public PduListener, public ExpiredPduListener,
public SendSubscriber, public smsc::core::threads::Thread
{
public:
    explicit SmppTestCore( unsigned expireTmo ) :
    log_(smsc::logger::Logger::getInstance("smpp.core")),
    registry_(*this,expireTmo),
    prevtime_(currentTimeMillis()),
    stopping_(true)
    {
        cnt_ = new smsc::core::synchronization::AtomicCounter<unsigned>[4];
    }


    ~SmppTestCore() {
        stop();
        delete [] cnt_;
    }


    void registerPdu( Pdu* pdu ) {
        registry_.registerPdu(pdu);
    }


    Pdu* unregisterPdu( uint32_t seqNum ) {
        return registry_.unregisterPdu(seqNum);
    }


    void setSocketId( uint32_t seqNum, unsigned socketId ) {
        registry_.setSocketId(seqNum,socketId);
    }


    virtual void socketClosed( unsigned sockid, Pdu* pdu )
    {
        if (!pdu) return;
        char buf[200];
        smsc_log_debug(log_,"pdu %s Sk%u closed",pdu->toString(buf,sizeof(buf)),sockid);
        delete pdu;
    }


    virtual void pduExpired( Pdu* pdu )
    {
        if (!pdu) return;
        char buf[200];
        smsc_log_debug(log_,"pdu %s expired",pdu->toString(buf,sizeof(buf)));
        delete pdu;
    }


    virtual void receivePdu( Socket& socket, const PduInputBuf& inbuf )
    {
        std::auto_ptr< Pdu > pdu(inbuf.decode());
        if ( pdu->isRequest() ) {
            cnt_[0].inc();
            char buf[200];
            smsc_log_debug(log_,"Sk%u received request %s",
                           socket.getSocketId(),pdu->toString(buf,sizeof(buf)));
            std::auto_ptr< ResponsePdu > resp(pdu->createResponse());
            if (resp.get()) {
                socket.send(resp->encode(),0);
            }
        } else {
            cnt_[1].inc();
            char buf[200];
            smsc_log_debug(log_,"Sk%u received resp %s",
                           socket.getSocketId(),pdu->toString(buf,sizeof(buf)));
            delete registry_.unregisterPdu(pdu->getSeqNum());
        }
    }


    virtual void wasSent( Socket& socket,
                          uint32_t seqNum,
                          bool isRequest )
    {
        smsc_log_debug(log_,"Sk%u sent %s seq=%u",
                       socket.getSocketId(),
                       isRequest ? "request" : "response",
                       seqNum );
        cnt_[isRequest ? 2 : 3].inc();
    }


    /// pdu is failed to be sent
    /// @param exc details of failure (may be missing)
    virtual void failedToSend( Socket& socket,
                               uint32_t seqNum,
                               bool isRequest,
                               const std::exception* exc )
    {
        smsc_log_debug(log_,"Sk%u failed to send %s seq=%u: %s",
                       socket.getSocketId(),
                       isRequest ? "request" : "response",
                       seqNum,
                       exc ? exc->what() : "unknown" );
        delete registry_.unregisterPdu(seqNum);
    }


    void start()
    {
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            if (!stopping_) return;
            stopping_ = false;
            mon_.notify();
        }
        registry_.start();
        Start();
    }


    void stop()
    {
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            if (stopping_) return;
            stopping_ = true;
            mon_.notify();
        }
        registry_.stop();
        WaitFor();
    }


    virtual int Execute()
    {
        smsc_log_info(log_,"started");
        msectime_type printtime = 2000;
        unsigned prevcnt[4] = {0,};
        while (!stopping_) {
            
            msectime_type now = currentTimeMillis();
            const msectime_type delta = now - prevtime_;
            {
                smsc::core::synchronization::MutexGuard mg(mon_);
                if ( delta < printtime ) {
                    mon_.wait(int(printtime-delta));
                    continue;
                }
            }

            const double divc = double(delta)/1000; // in seconds
            double speed[4];
            for ( unsigned i = 0; i < 4; ++i ) {
                const unsigned curcnt = cnt_[i].get();
                speed[i] = (curcnt-prevcnt[i]) / divc;
                prevcnt[i] = curcnt;
            }
            smsc_log_info(log_,"received origs=%u(%.3g) resps=%u(%.3g)",
                          prevcnt[0], speed[0], prevcnt[1], speed[1]);
            printf("received origs=%u(%.3g) resps=%u(%.3g)\n",
                   prevcnt[0], speed[0], prevcnt[1], speed[1]);
            prevtime_ = now;
        }
        smsc_log_info(log_,"finished");
        return 0;
    }

private:
    smsc::logger::Logger* log_;
    RespRegistry          registry_;
    smsc::core::synchronization::EventMonitor mon_;
    
    // NOTE: we use dynamic allocation because of
    // brain-dead sunos compiler leads to sigsegv on silverstone when using array.
    smsc::core::synchronization::AtomicCounter<unsigned>* cnt_;
    // smsc::core::synchronization::AtomicCounter<unsigned> cnt_[4];
    msectime_type prevtime_;
    bool stopping_;
};

}
}

#endif
