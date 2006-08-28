#ident "$Id$"
#ifndef __SMSC_SNMP_TRAPRECORD_HPP__
#define __SMSC_SNMP_TRAPRECORD_HPP__

#include "SnmpAgent.hpp"
namespace smsc  {
namespace snmp {
using smsc::snmp::SnmpAgent;

struct TrapRecord {
    TrapRecord();

    //encodes trap to CSV format
    static void  csvEncode(const TrapRecord & cdr, std::string & rec);
    static const char* headerText();

    time_t          submitTime;       //SUBMIT: sms submit time
    std::string     alarmId;          //SRC_ADDR: sender number
    std::string     alarmObjCategory; //SRC_IMSI: sender IMSI
    SnmpAgent::alertSeverity   severity;         //SRC_MSC: sender MSC
    std::string     text;             //SRC_SME_ID: sender SME identifier
};


} //snmp
} //smsc

#endif /* __SMSC_SNMP_TRAPRECORD_HPP__ */
