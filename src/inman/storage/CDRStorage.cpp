#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/storage/CDRStorage.hpp"

using smsc::inman::cdr::_CDRRecordHeader_TEXT;

namespace smsc {
namespace inman {
namespace filestore {

/* ************************************************************************** *
 * class InBillingFileStorage implementation:
 * ************************************************************************** */
static const RollingFileStorageParms _smsc_BILLING_STORAGE_parms = {
    _CDRRecordHeader_TEXT, 0, 0, NULL
};
static const char* _smsc_LAST_BILLING_FILE_EXTENSION = "lst";
static const char* _smsc_BILLING_FILE_EXTENSION = "csv";

InBillingFileStorage::InBillingFileStorage(const std::string & location,
                                            unsigned long rollInterval/* = 0*/,
                                            Logger * uselog /* = NULL */)
    : InRollingFileStorage(location,
                           _smsc_LAST_BILLING_FILE_EXTENSION,
                           _smsc_BILLING_FILE_EXTENSION, rollInterval,
                            &_smsc_BILLING_STORAGE_parms, uselog)
{ }

InBillingFileStorage::~InBillingFileStorage()
{ }

void InBillingFileStorage::bill(const CDRRecord & cdr)
{
    std::string rec;
    CDRRecord::csvEncode(cdr, rec);
    smsc_log_debug(logger, "BFS: Writing TDR: msgId = %llu, rec sz = %d", cdr._msgId,  rec.size());
    RFSWrite(rec.c_str(), rec.size());
    RFSFlush();
}

} //filestore
} //inman
} //smsc

