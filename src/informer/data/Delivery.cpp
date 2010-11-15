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
userInfo_(&userInfo),
activityLog_(*dlvInfo_.get()),
source_(source),
ref_(0),
state_(DlvState(0)),
planTime_(0)
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
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    if ( texts.size() > 300 ) {
        throw InfosmeException(EXC_SYSTEM,"D=%u too long glossary requested",dlvId);
    }
    MessageGlossary::TextList tl;
    int32_t id = 0;
    for ( std::vector< std::string >::const_iterator i = texts.begin();
          i != texts.end(); ++i ) {
        tl.push_back( new MessageText( i->c_str(), ++id) );
    }
    source_->getGlossary().setTexts( dlvInfo_->getCS().getStorePath(), dlvId, tl );
}

}
}
