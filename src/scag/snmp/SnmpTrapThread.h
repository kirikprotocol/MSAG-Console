#ifndef _SCAG_SNMP_SNMPTRAPTHREAD_H
#define _SCAG_SNMP_SNMPTRAPTHREAD_H

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "SnmpWrapper.h"

namespace scag2 {
namespace snmp {

class SnmpTrapThread : public smsc::core::threads::Thread, public TrapRecordQueue
{
private:
    typedef smsc::core::buffers::FastMTQueue< TrapRecord* > QueueType;
    
public:
    SnmpTrapThread( SnmpWrapper* snmp ) :
    smsc::core::threads::Thread(), stopping_(true), stopped_(true), snmp_(snmp), log_(0) 
    {
        log_ = smsc::logger::Logger::getInstance("snmp.trap");
    }
    virtual ~SnmpTrapThread();
    virtual int Execute();
    void Start() {
        stopped_ = stopping_ = false;
        smsc::core::threads::Thread::Start();
    }
    void Stop() {
        if ( ! stopping_ ) {
            stopping_ = true;
            queue_.notify();
        }
    }

    virtual void Push( TrapRecord* trap ) {
        if ( stopping_ ) delete trap;
        else queue_.Push( trap );
    }
    // inline TrapRecordQueue* getQueue() { return this; }

private:
    void waitStop();

    void Start(int) { Start(); }
    SnmpTrapThread();

private:
    bool                              stopping_;
    bool                              stopped_;
    SnmpWrapper*                      snmp_; // not owned
    QueueType                         queue_;
    smsc::logger::Logger*             log_;
    smsc::core::synchronization::EventMonitor stopMon_;
};

}
}

#endif /* !_SCAG_SNMP_SNMPTRAPTHREAD_H */
