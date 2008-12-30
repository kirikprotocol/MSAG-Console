#ifndef _SCAG_PVSS_CLIENT_PVSSWRITER_H
#define _SCAG_PVSS_CLIENT_PVSSWRITER_H

#include "PvssIOTask.h"
#include "PvssConnection.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssWriter : public PvssIOTask
{
public:
    PvssWriter( PvssStreamClient& pers,
                smsc::core::synchronization::EventMonitor* queueMon ) :
    PvssIOTask(pers), queuemon_(queueMon) {}
    virtual const char* taskName() { return "PvssWriter"; }
    
protected:
    virtual bool setupSocket( PvssConnection& conn ) {
        if ( ! conn.wantToSend() ) return false;
        mul_.add( conn.socket() );
        return true;
    }
    virtual void setupFailed() {
        smsc::core::synchronization::MutexGuard mg(*queuemon_);
        queuemon_->wait(500);
    }
    virtual bool hasEvents() { return mul_.canWrite(ready_, error_, 200); }
    virtual void process( PvssConnection& con ) { con.sendData(); }

private:
    smsc::core::synchronization::EventMonitor* queuemon_;
};


} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSWRITER_H */
