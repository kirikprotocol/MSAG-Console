#include "NotificationManager.h"

namespace scag2 {
namespace counter {
namespace impl {

NotificationManager::NotificationManager() :
log_(smsc::logger::Logger::getInstance("cnt.nmgr")),
serial_(0)
{
    smsc_log_debug(log_,"ctor");
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
            smsc_log_debug(log_,"sending TRAP '%s' level=%s val=%lld",
                           c, severityToString(s->severity), s->value );
        } else {
            // removing obsolete elements from hash
            smsc_log_debug(log_,"trap '%s' level=%s val=%lld is obsolete, deleting",
                           c, severityToString(s->severity), s->value );
            sendDataHash_.Delete(c);
        }
    }
    ++serial_;
}


void NotificationManager::notify( const char* c, CntSeverity& sev,
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
        if ( (limit.optype == LT) == (value < ptr->value) ) { // XOR
            ptr->value = value;
        }
    }
}

}
}
}
