#ifndef _SCAG_SNMP_TRAPRECORD_H
#define _SCAG_SNMP_TRAPRECORD_H

#include <time.h>
#include "core/buffers/FixedLengthString.hpp"

namespace scag2 {
namespace snmp {

/// NOTE: we use our own TrapRecord to decouple from smsc::snmp
/// which has too many dependencies on (unneeded for us) smsc libraries

struct TrapRecord
{
public:
    TrapRecord() : submitTime(time(0)) {}

    enum RecordType {
            Trap = 1,
            Notification,
            StatusChange
    };
    enum TrapStatus {
            STATNEW = 1,
            STATCLEAR
    };
    enum Severity {
            CLEAR = 1,
            NORMAL,
            MINOR,
            MAJOR,
            CRITICAL
    };

public:
    RecordType   recordType;
    time_t       submitTime;
    TrapStatus   status;     // this determine which kind of message we'll use (alarm, clear, etc)
    smsc::core::buffers::FixedLengthString<32> id;
    smsc::core::buffers::FixedLengthString<32> category;
    Severity     severity;
    std::string  text;

    // typedef smsc::core::buffers::CyclicQueue< TrapRecord* >  QueueType;
};


// an interface of a trap record queue
class TrapRecordQueue
{
public:
    virtual void Push( TrapRecord* trap ) = 0;
    // virtual TrapRecord* Pop() = 0;
};

}
}

namespace scag {
namespace snmp = scag2::snmp;
}

#endif /* !_SCAG_SNMP_TRAPRECORD_H */
