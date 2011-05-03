#include <cassert>
#include "NotificationManager.h"
#include "scag/snmp/TrapRecord.h"
#include "core/buffers/TmpBuf.hpp"

namespace scag2 {
namespace counter {
namespace impl {

using namespace scag2::snmp;

NotificationManager::NotificationManager() :
log_(smsc::logger::Logger::getInstance("cnt.nmgr")),
serial_(0)
{
    smsc_log_debug(log_,"ctor");
    initMapping();
}


NotificationManager::~NotificationManager()
{
    smsc_log_debug(log_,"dtor");
    /*
    Counter* c;
    SendData* s;
    for ( SendDataHash::Iterator i(&sendDataHash_); i.Next(c,s); ) {
        delete s;
    }
     */
}


void NotificationManager::advanceTime( usec_type curTime )
{
    smsc_log_debug(log_,"advanceTime %lld", curTime);
    MutexGuard mg(lock_);
    char* c;
    SendData* s;
    for ( SendDataHash::Iterator i(&sendDataHash_); i.Next(c,s); ) {
        if ( s->serial == serial_ ) {
#ifdef SNMP
            TrapRecordQueue* inst = TrapRecordQueue::getInstance();
            TrapData* trd;
            const char* idfound;
            if (inst && (trd=findTrapData(c,&idfound)) ) {
                smsc_log_debug(log_,"sending TRAP '%s' level=%s val=%lld",
                               c, severityToString(s->severity), s->value );
                TrapRecord* rec = new TrapRecord;
                rec->recordType = TrapRecord::Trap;
                rec->submitTime = curTime/usecFactor;
                rec->status = TrapRecord::TrapType(trd->trap); // the type of trap
                rec->id = idfound;
                rec->category = trd->cat;
                rec->severity = TrapRecord::Severity(s->severity);
                char buf[200];
                snprintf(buf,sizeof(buf),"%s=%lld",c,s->value);
                rec->text = buf;
                inst->Push(rec);
            }
#endif
        } else {
            // removing obsolete elements from hash
            smsc_log_debug(log_,"trap '%s' level=%s val=%lld is obsolete, deleting",
                           c, severityToString(s->severity), s->value );
            sendDataHash_.Delete(c);
        }
    }
    ++serial_;
}


void NotificationManager::notifyOnLimit( const char* c, CntSeverity& sev,
                                         int64_t value, const ActionLimit& limit )
{
    smsc_log_debug(log_,"notifying '%s' value=%lld %s limit=%lld status=%s",
                   c, value, opTypeToString(limit.optype),
                   limit.limit, severityToString(limit.severity));
    SendData* ptr;
    MutexGuard mg(lock_);
    ptr = sendDataHash_.GetPtr(c);
    if ( !ptr ) {
        // inserting
        sendDataHash_.Insert(c,SendData(value,limit.severity,serial_));
        if (sev < limit.severity) sev = limit.severity;
    } else if ( ptr->serial != serial_ ) {
        // obsolete, replacing
        ptr->value = value;
        sev = ptr->severity = limit.severity;
        ptr->serial = serial_;
    } else if ( ptr->severity > limit.severity ) {
        // lower level, do not merge
    } else if ( ptr->severity < limit.severity ) {
        // higher level, simply replace
        ptr->value = value;
        sev = ptr->severity = limit.severity;
    } else {
        // merging
        if ( (limit.optype == OPTYPELE) == (value < ptr->value) ) { // XOR
            ptr->value = value;
        }
    }
}


void NotificationManager::initMapping()
{
    mappingHash_.Insert("sys.traffic.global.smpp",TrapData(TrapRecord::TRAPTTRAFFIC,"SMPP"));
    mappingHash_.Insert("sys.traffic.smpp.sme",TrapData(TrapRecord::TRAPTSMPPTRAF,"SME"));
    mappingHash_.Insert("sys.traffic.smpp.smsc",TrapData(TrapRecord::TRAPTSMPPTRAF,"SMSC"));
    mappingHash_.Insert("sys.smpp.queue.global",TrapData(TrapRecord::TRAPTSMPPQLIM,"TOTAL"));
    mappingHash_.Insert("sys.smpp.queue.in",TrapData(TrapRecord::TRAPTSMPPQLIM,"IN"));
    mappingHash_.Insert("sys.smpp.queue.out",TrapData(TrapRecord::TRAPTSMPPQLIM,"OUT"));
    mappingHash_.Insert("sys.sessions.total",TrapData(TrapRecord::TRAPTSESSLIM,"TOTAL"));
    mappingHash_.Insert("sys.sessions.active",TrapData(TrapRecord::TRAPTSESSLIM,"ACTIVE"));
    mappingHash_.Insert("sys.sessions.locked",TrapData(TrapRecord::TRAPTSESSLIM,"LOCKED"));
}


NotificationManager::TrapData* NotificationManager::findTrapData( const char* cname,
                                                                  const char** idfound )
{
    const size_t cnameLen = strlen(cname);
    if (idfound) *idfound = cname + cnameLen;
    // try to find the name as a whole
    TrapData* ptr = mappingHash_.GetPtr(cname);
    if (ptr) {
        return ptr;
    }
    const char* p = cname + cnameLen;
    const char delim = '.';
    while (true) {
        for ( --p; p>cname && *p!=delim; --p ) {
        }
        if ( p<=cname ) {
            return 0;
        }
        assert(*p==delim);
        smsc::core::buffers::TmpBuf<char,100> temp;
        const size_t pfxlen = p-cname;
        memcpy(temp.setSize(pfxlen+1),cname,pfxlen);
        temp.get()[pfxlen] = '\0';
        ptr = mappingHash_.GetPtr(temp.get());
        if (ptr) {
            if (idfound) *idfound = cname+pfxlen+1;
            return ptr;
        }
    }
}

}
}
}
