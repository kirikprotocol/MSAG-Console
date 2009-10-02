#include <cassert>
#include "SnmpTrapThread.h"

namespace scag2 {
namespace snmp {

SnmpTrapThread::~SnmpTrapThread()
{
    smsc_log_info(log_,"snmp trap thread dtor started");
    Stop();
    waitStop();
    unsigned count = 0;
    TrapRecord* r;
    while ( queue_.Pop(r) ) {
        ++count;
        delete r;
    }
    if ( count > 0 ) {
        smsc_log_warn( log_, "queue still had %u elements to send, all were dropped", count );
    }
}


int SnmpTrapThread::Execute()
{
    stopped_ = false;
    smsc_log_info( log_, "snmp trap thread is started" );
    TrapRecord* tr;
    while ( ! stopping_ ) {
        if ( ! queue_.Pop(tr) ) {
            queue_.Wait();
            continue;
        }
        if ( !tr ) continue;
        snmp_->sendTrap(*tr);
    }
    // snmp should be still valid here
    while ( queue_.Pop(tr) ) {
        if ( !tr ) continue;
        snmp_->sendTrap(*tr);
    }
    smsc_log_info( log_, "snmp trap thread is stopped" );
    MutexGuard mg(stopMon_);
    stopped_ = true;
    stopMon_.notify();
    return 0;
}


void SnmpTrapThread::waitStop()
{
    MutexGuard mg(stopMon_);
    while ( ! stopped_ ) {
        stopMon_.wait(500);
    }
}

}
}
