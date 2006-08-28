#ident "$Id$"
#ifndef __SMSC_SNMP_TRAPRECORDLOG_HPP__
#define __SMSC_SNMP_TRAPRECORDLOG_HPP__

#include "inman/storage/FileStorages.hpp"

#include "TrapRecord.hpp"
using smsc::snmp::TrapRecord;

namespace smsc {
namespace snmp {
using smsc::inman::filestore::InRollingFileStorage;

class TrapRecordLog : public InRollingFileStorage
{
public:
    TrapRecordLog(const std::string & location, unsigned long rollInterval = 0, Logger * uselog  = NULL);
    ~TrapRecordLog();

    void log(const TrapRecord & trap);
};

} //snmp
} //smsc

#endif /* __SMSC_SNMP_TRAPRECORDLOG_HPP__ */
