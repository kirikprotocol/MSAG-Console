#include <cassert>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "SnmpTrapThread.h"
#include "util/TimeSource.h"
#include "SnmpAgent.hpp"

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
    unsigned pingTimeout = pingTimeout_;
    TrapRecord ping;
    ping.recordType = TrapRecord::Trap;
    ping.status = TrapRecord::TRAPTALIVE;
    if ( snmp_->isMsag() ) {
        ping.id = "MSAG";
        ping.category = "MSAG";
    } else {
        pingTimeout = 0;
    }
    ping.severity = TrapRecord::CLEAR;
    ping.text = "I'm alive";

    smsc_log_info( log_, "snmp trap thread is started%s",
                   pingTimeout ? "" : " (pings disabled)");
    TrapRecord* tr;
    typedef smsc::util::TimeSourceSetup::AbsUSec TSource;
    typedef TSource::usec_type usec_type;
    usec_type lastping = TSource::getUSec();

// from smsc snmpAgent:
    struct timeval t;t.tv_sec=0,t.tv_usec=500000;
    snmp_alarm_register_hr(t, SA_REPEAT, dummyAlert, 0);

    while ( ! stopping_ ) {
        agent_check_and_process(1);
        while ( queue_.Pop(tr) ) {
            snmp_->sendTrap(*tr);
        }
        const usec_type now = TSource::getUSec();
        const int delta = int((now - lastping)/1000000);
        if ( pingTimeout && delta > int(pingTimeout) ) {
            snmp_->sendTrap(ping);
            lastping = now;
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
    smsc::core::synchronization::MutexGuard mg(stopMon_);
    stopped_ = true;
    stopMon_.notify();
    return 0;
}


void SnmpTrapThread::waitStop()
{
    smsc::core::synchronization::MutexGuard mg(stopMon_);
    while ( ! stopped_ ) {
        stopMon_.wait(500);
    }
}

}
}
