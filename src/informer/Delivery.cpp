#include "Delivery.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
                    StoreJournal&  storeLog,
                    InputMessageSource& messageSource ) :
dlvInfo_(dlvInfo),
cache_(* dlvInfo_.get(), storeLog, messageSource )
{
}

}
}
