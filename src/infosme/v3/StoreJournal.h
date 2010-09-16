#ifndef _INFOSME_V3_STOREJOURNAL_H
#define _INFOSME_V3_STOREJOURNAL_H

namespace smsc {
namespace infosme {

/// two functionality:
/// 1. journalling all activity on the working storage;
/// 2. restore working storage state on restart.
class StoreJournal
{
public:
    void journalMessage( dlvid_type dlvId,
                         regionid_type regionId,
                         Message& msg );
};

} // infosme
} // smsc

#endif
