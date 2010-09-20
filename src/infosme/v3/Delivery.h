#ifndef _INFOSME_V3_DELIVERY_H
#define _INFOSME_V3_DELIVERY_H

#include <memory>
#include "MessageCache.h"

namespace smsc {
namespace infosme {

class StoreJournal;
class InputMessageSource;

class Delivery
{
public:
    Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
              StoreJournal&  storeLog,
              InputMessageSource& messageSource );

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    bool isActive() const {
        // FIXME
        return true;
    }
    unsigned getPriority() const { return dlvInfo_->getPriority(); }

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId ) {
        return cache_.getRegionalStorage(regId);
    }

public:
    std::auto_ptr<DeliveryInfo> dlvInfo_;
    MessageCache                cache_;
};

} // infosme
} // smsc

#endif
