#include <cassert>
#include "Delivery.h"
#include "CommonSettings.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( DeliveryInfo*               dlvInfo,
                    InputMessageSource*         source ) :
log_(0),
dlvInfo_(dlvInfo),
activityLog_(*dlvInfo_.get()),
source_(source),
ref_(0)
{
    char buf[20];
    sprintf(buf,"dl.%u",dlvInfo_->getDlvId());
    log_ = smsc::logger::Logger::getInstance(buf);
    try {
        source_->init(activityLog_);
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"D=%u inputstorage init failed: %s",
                       dlvInfo_->getDlvId(), e.what());
        delete source_;
        throw;
    }
}


Delivery::~Delivery()
{
    smsc_log_info(log_,"dtor D=%u",dlvInfo_->getDlvId());
    delete source_;
}


void Delivery::updateDlvInfo( const DeliveryInfoData& infoData )
{
    assert( dlvInfo_.get() );
    dlvInfo_->update( infoData );
}

}
}
