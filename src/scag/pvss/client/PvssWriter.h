#ifndef _SCAG_PVSS_CLIENT_PVSSWRITER_H
#define _SCAG_PVSS_CLIENT_PVSSWRITER_H

#include "PvssIOTask.h"
#include "PvssConnection.h"
#include "PvssStreamClient.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssWriter : public PvssIOTask
{
public:
    PvssWriter( PvssStreamClient& pers ) :
    PvssIOTask(pers,"pvss.wio") {}
    virtual const char* taskName() { return "PvssWriter"; }
    
    /// is used from pvssreader in synchronous mode
    /// when a connection becomes available i.e. when it finishes reading.
    void notify() {
        smsc::core::synchronization::MutexGuard mg(mon_);
        mon_.notify();
    }

protected:
    virtual bool setupSockets()
    {
        hasAvailable_ = false;
        return PvssIOTask::setupSockets();
    }

    virtual bool setupSocket( PvssConnection& conn ) {
        if ( ! pers_->async && conn.isReading() ) return false;
        hasAvailable_ = true;
        if ( ! conn.wantToSend() ) return false;
        mul_.add( conn.socket() );
        return true;
    }
    virtual void setupFailed() {
        // smsc_log_debug( log_, "setup failed: avail=%d", hasAvailable_ ? 1 : 0 );
        if ( hasAvailable_ ) {
            pers_->waitForCalls(200);
        } else if ( ! pers_->async ) {
            smsc::core::synchronization::MutexGuard mg(mon_);
            hasAvailable_ = false;
            for ( int i = 0; i < sockets_.Count(); ++i ) {
                if ( ! sockets_[i]->isReading() ) {
                    hasAvailable_ = true;
                    break;
                }
            }
            if ( ! hasAvailable_ && sockets_.Count() > 0 ) {
                mon_.wait(200);
            }
        }
    }
    virtual bool hasEvents() { return mul_.canWrite(ready_, error_, 200); }
    virtual void process( PvssConnection& con ) { con.sendData(); }

private:
    // smsc::core::synchronization::EventMonitor* queuemon_;
    bool hasAvailable_;
};


} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSWRITER_H */
