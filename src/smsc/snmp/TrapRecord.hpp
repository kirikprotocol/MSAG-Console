#ident "$Id$"
#ifndef __SMSC_SNMP_TRAPRECORD_HPP__
#define __SMSC_SNMP_TRAPRECORD_HPP__

#include "core/buffers/FixedLengthString.hpp"

#include "SnmpAgent.hpp"

namespace smsc  {
namespace snmp {
using smsc::snmp::SnmpAgent;

struct TrapRecord {
    TrapRecord();

    //encodes trap to CSV format
    static void  csvEncode(const TrapRecord & cdr, std::string & rec);
    static const char* headerText();

    enum RecordType{
      rtTrap,rtNotification,rtStatusChange
    };
    RecordType recordType;
    time_t submitTime;
    int status;
    smsc::core::buffers::FixedLengthString<32> alarmId;
    smsc::core::buffers::FixedLengthString<32> alarmObjCategory;
    SnmpAgent::alertSeverity                   severity;
    std::string                                text;
};


} //snmp
} //smsc

#endif /* __SMSC_SNMP_TRAPRECORD_HPP__ */
