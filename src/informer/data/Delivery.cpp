#include <cassert>
#include "Delivery.h"
#include "UserInfo.h"
#include "CommonSettings.h"
#include "MessageGlossary.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( DeliveryInfo*               dlvInfo,
                    UserInfo&                   userInfo,
                    InputMessageSource*         source ) :
log_(0),
dlvInfo_(dlvInfo),
state_(DlvState(0)),
planTime_(0),
activityLog_(userInfo,dlvInfo_->getDlvId()),
source_(source),
ref_(0)
{
    char buf[20];
    sprintf(buf,"d.%u",dlvInfo_->getDlvId() % 10000);
    log_ = smsc::logger::Logger::getInstance(buf);
    try {
        source_->init(activityLog_);
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"D=%u inputstorage init failed, exc: %s",
                       dlvInfo_->getDlvId(), e.what());
        delete source_;
        throw;
    }
}


Delivery::~Delivery()
{
    delete source_;
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    dlvInfo_.reset(0);
    smsc_log_info(log_,"dtor D=%u done",dlvId);
}


void Delivery::updateDlvInfo( const DeliveryInfoData& infoData )
{
    assert( dlvInfo_.get() );
    dlvInfo_->update( infoData );
}


void Delivery::getGlossary( std::vector< std::string >& texts ) const
{
    source_->getGlossary().getTexts( texts );
}


void Delivery::setGlossary( const std::vector< std::string >& texts )
{
    source_->getGlossary().setTexts( texts );
}

}
}
