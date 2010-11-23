static char const ident[] = "$Id$";

#include "TrapRecordLog.hpp"

namespace smsc {
namespace snmp {

/* ************************************************************************** *
 * class TrapRecordLog implementation:
 * ************************************************************************** */
static const smsc::inman::filestore::RollingFileStorageParms _smsc_snmp_STORAGE_parms = {
    TrapRecord::headerText(), 0, 0, NULL
};
TrapRecordLog::TrapRecordLog(const std::string & location,
                             const char *lastExt, const char *storageExt,
                             unsigned long rollInterval, Logger * uselog)
    : InRollingFileStorage(location,lastExt,storageExt, rollInterval,&_smsc_snmp_STORAGE_parms, uselog)
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
