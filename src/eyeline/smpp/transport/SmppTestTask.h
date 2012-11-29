#ifndef _EYELINE_SMPP_SMPPTESTTASK_H
#define _EYELINE_SMPP_SMPPTESTTASK_H

#include "SmppTestCore.h"
#include "SessionBase.h"
#include "core/threads/ThreadedTask.hpp"
#include "informer/io/SpeedControl.h"

namespace eyeline {
namespace smpp {

/// This class is for testing purposes only.
/// see client/testClient server/testServer.
class SmppTestTask : public smsc::core::threads::ThreadedTask
{
    static smsc::logger::Logger* log_;
public:

    SmppTestTask( SmppTestCore& core,
                  unsigned speed ) :
    core_(core)
    {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("smpp.stm");
        }
        speed_.setSpeed(speed,currentTimeMillis());
    }


    virtual const char* taskName() {
        return "smpp.stm";
    }


    virtual int Execute()
    {
        smsc_log_info(log_,"started");
        SessionPtr session;
        while ( ! isStopping ) {

            // check speed
            const msectime_type now = currentTimeMillis();
            const msectime_type delta = speed_.isReady(now,10000);
            if ( delta > 0 ) {
                timespec ts;
                ts.tv_sec = delta / 1000;
                ts.tv_nsec = (delta % 1000) * 1000000;
                nanosleep(&ts,0);
            }

            if ( !getSession(session) ) {
                speed_.suspend(now+1000);
                continue;
            }

            if ( !session->isBound() ) {
                speed_.suspend(now+1000);
                continue;
            }

            // sending a selection of pdu
            std::auto_ptr<Pdu> pdu;
            try {
                pdu.reset(makePdu());
            } catch ( std::exception& e ) {
                smsc_log_debug(log_,"failed to create pdu: %s",e.what());
                speed_.suspend(now+1000);
                continue;
            }

            while ( pdu.get() ) {

                PduBuffer pbuf;
                try {
                    pbuf = pdu->encode();
                } catch ( std::exception& e ) {
                    char buf[200];
                    smsc_log_warn(log_,"pdu=%s cannot be encoded: %s",
                                  pdu->toString(buf,sizeof(buf)),e.what());
                    speed_.suspend(now+500);
                    break;
                }
                const uint32_t seqNum = pdu->getSeqNum();

                try {
                    core_.registerPdu(pdu.release());
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"pdu seq=%u cannot be registered: %s",seqNum,e.what());
                    speed_.suspend(now+500);
                    break;
                }

                try {
                    const unsigned sockid = session->send(0,pbuf,&core_);
                    core_.setSocketId(seqNum,sockid);

                    // successfully sent, sleeping
                    speed_.consumeQuant();

                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"pdu seq=%u cannot be sent: %s",seqNum,e.what());
                    delete core_.unregisterPdu(seqNum);
                    speed_.suspend(now+500);
                }
                break;
            } 
        }
        smsc_log_info(log_,"finished");
        return 0;
    }

protected:
    virtual Pdu* makePdu() = 0;
    virtual bool getSession( SessionPtr& ptr ) = 0;

private:
    SmppTestCore&  core_;
    eyeline::informer::SpeedControl<msectime_type> speed_;
};

}
}

#endif
