#ifndef _SCAG_PROTOTYPES_INFOSME_MESSAGECACHE_H
#define _SCAG_PROTOTYPES_INFOSME_MESSAGECACHE_H

#include "RegionalStorage.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntHash.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

class RegionalStoragePtr;

/// the cache of messages for the task.
class MessageCache
{
public:
    MessageCache( const TaskInfo& taskInfo );

    /// get the regional storage for given region.
    RegionalStoragePtr getRegionalStorage( regionid_type regionId );

    /// rolling over the storage.
    void rollOver();

    /// add a new regional storage.
    void addRegionalStorage( RegionalStoragePtr storage );

    const TaskInfo& getTaskInfo() const { return *taskInfo_; }

private:
    smsc::core::synchronization::Mutex                 cacheLock_;
    smsc::core::buffers::IntHash< RegionalStoragePtr > storages_;
    const TaskInfo*                                    taskInfo_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
