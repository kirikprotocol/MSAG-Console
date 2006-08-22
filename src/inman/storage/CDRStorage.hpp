#ident "$Id$"
#ifndef _INMAN_CDR_STORAGE_H
#define _INMAN_CDR_STORAGE_H

#include "inman/storage/FileStorages.hpp"

#include "inman/storage/cdrutil.hpp"
using smsc::inman::cdr::CDRRecord;

namespace smsc {
namespace inman {
namespace filestore {

class InBillingFileStorage : public InRollingFileStorage 
{
public:
    InBillingFileStorage(const std::string & location, unsigned long rollInterval = 0, Logger * uselog  = NULL);
    ~InBillingFileStorage();

    void bill(const CDRRecord & cdr);
};

} //filestore
} //inman
} //smsc

#endif /* _INMAN_CDR_STORAGE_H */


