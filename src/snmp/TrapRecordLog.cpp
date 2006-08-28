static char const ident[] = "$Id$";

#include "TrapRecordLog.hpp"

namespace smsc {
namespace snmp {

/* ************************************************************************** *
 * class TrapRecordLog implementation:
 * ************************************************************************** */
static const RollingFileStorageParms _smsc_snmp_STORAGE_parms = {
    TrapRecord::headerText(), 0, 0, NULL
};
static const char* _smsc_snmp_CURRENT_LOG_FILE_EXTENSION = "lst";
static const char* _smsc_snmp_ARCHIVED_LOG_FILE_EXTENSION = "csv";

TrapRecordLog::TrapRecordLog(const std::string & location,
                                            unsigned long rollInterval/* = 0*/,
                                            Logger * uselog /* = NULL */)
    : InRollingFileStorage(location,
                           _smsc_snmp_CURRENT_LOG_FILE_EXTENSION,
                           _smsc_snmp_ARCHIVED_LOG_FILE_EXTENSION, rollInterval,
                            &_smsc_snmp_STORAGE_parms, uselog)
{ }

TrapRecordLog::~TrapRecordLog()
{ }

void TrapRecordLog::log(const TrapRecord & cdr)
{
    std::string rec;
    TrapRecord::csvEncode(cdr, rec);
    RFSWrite(rec.c_str(), rec.size());
    RFSFlush();
}

} //snmp
} //smsc
