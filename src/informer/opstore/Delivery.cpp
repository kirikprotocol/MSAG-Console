#include <cassert>
#include "Delivery.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
                    StoreJournal&               journal,
                    InputMessageSource*         source ) :
dlvInfo_(dlvInfo),
cache_(*dlvInfo_.get(),journal,source),
ref_(0)
{
}


void Delivery::updateDlvInfo( const DeliveryInfo& info )
{
    assert( dlvInfo_.get() );
    assert(dlvInfo_->getDlvId() == info.getDlvId());
    // FIXME: update dlvinfo
    //*dlvInfo_.get() = info;
}


void Delivery::setRecordAtInit( regionid_type            regionId,
                                const InputRegionRecord& rec,
                                uint64_t                 maxMsgId )
{
    cache_.getInputSource().setRecordAtInit(regionId,rec,maxMsgId);
}


void Delivery::setRecordAtInit( regionid_type            regionId,
                                Message&                 msg,
                                regionid_type            serial )
{
    cache_.setRecordAtInit(regionId,msg,serial);
}


void Delivery::postInitInput( std::vector< regionid_type >& filledRegs )
{
    cache_.getInputSource().postInit(filledRegs);
}

}
}
