#ifndef _INMAN_CDR_STORAGE_H
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _INMAN_CDR_STORAGE_H

#include "inman/storage/FileStorages.hpp"
#include "inman/storage/cdrutil.hpp"

namespace smsc {
namespace inman {
namespace filestore {

using smsc::inman::cdr::CDRRecord;

class InBillingFileStorage : public InRollingFileStorage {
public:
  InBillingFileStorage(const std::string & location, unsigned long rollInterval = 0, Logger * uselog  = NULL);
  ~InBillingFileStorage();

  void bill(const CDRRecord & cdr);
};

} //filestore
} //inman
} //smsc

#endif /* _INMAN_CDR_STORAGE_H */


