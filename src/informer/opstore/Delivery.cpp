#include <cassert>
#include "Delivery.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
                    StoreJournal&  storeLog,
                    InputMessageSource& messageSource ) :
dlvInfo_(dlvInfo),
cache_(* dlvInfo_.get(), storeLog, messageSource ),
ref_(0)
{
}


void Delivery::updateDlvInfo( const DeliveryInfo& info )
{
    assert( dlvInfo_.get() );
    assert(dlvInfo_->getDlvId() == info.getDlvId());
    // FIXME: update dlvinfo
    *dlvInfo_.get() = info;
}

}
}
