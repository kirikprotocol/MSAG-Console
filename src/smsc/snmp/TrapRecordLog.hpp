#ident "$Id$"
#ifndef __SMSC_SNMP_TRAPRECORDLOG_HPP__
#define __SMSC_SNMP_TRAPRECORDLOG_HPP__

#include "inman/storage/FileStorages.hpp"
#include "TrapRecord.hpp"

namespace smsc {
namespace snmp {

using smsc::inman::filestore::InRollingFileStorage;

class TrapRecordLog : public InRollingFileStorage
{
public:
    TrapRecordLog(const std::string & location, const char *lastExt, const char *storageExt, unsigned long rollInterval, Logger * uselog);
    ~TrapRecordLog();

    void log(const TrapRecord & trap);
};

} //snmp
} //smsc

#endif /* __SMSC_SNMP_TRAPRECORDLOG_HPP__ */
