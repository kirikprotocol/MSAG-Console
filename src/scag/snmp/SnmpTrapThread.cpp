#include <cassert>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
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
        agent_check_and_process(1);
        while ( queue_.Pop(tr) ) {
            snmp_->sendTrap(*tr);
        }
        if ( stopping_ ) break;
        // now, when we are processing snmp requests
        // we should not wait on the queue
        // queue_.Wait();
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
