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

public:
    std::auto_ptr<DeliveryInfo> dlvInfo_;
    MessageCache                cache_;
};

} // infosme
} // smsc

#endif
