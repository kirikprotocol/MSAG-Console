#include <cassert>
#include "Delivery.h"
#include "UserInfo.h"
#include "CommonSettings.h"
#include "MessageGlossary.h"
#include "ActivityLog.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( DeliveryInfo*               dlvInfo,
                    InputMessageSource*         source ) :
log_(0),
stateLock_( MTXWHEREAMI ),
state_(DlvState(0)),
planTime_(0),
dlvInfo_(dlvInfo),
activityLog_(0),
source_(source),
lock_( MTXWHEREAMI ),
ref_(0)
{
    char buf[20];
    sprintf(buf,"d.%02u",getDlvId() % 100);
    log_ = smsc::logger::Logger::getInstance(buf);
    if (source_) {
        try {
            activityLog_ = new ActivityLog(dlvInfo);
            source_->init(*activityLog_);
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"D=%u inputstorage init failed, exc: %s",
                           getDlvId(), e.what());
            delete source_;
            delete activityLog_;
            throw;
        }
    }
}


Delivery::~Delivery()
{
    delete source_;
    delete activityLog_;
    smsc_log_info(log_,"dtor D=%u done",getDlvId());
}


void Delivery::addNewMessages( MsgIter begin, MsgIter end )
{
    if ( !source_ ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    smsc_log_debug(log_," == addNewMessages(D=%u)", getDlvId());
    source_->addNewMessages(begin,end);
}


void Delivery::dropMessages( const std::vector< msgid_type >& msgids )
{
    if ( !source_ ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    }
    source_->dropMessages(msgids);
}


void Delivery::getGlossary( std::vector< std::string >& texts ) const
{
    // if (!source_) {
    // throw InfosmeException(EXC_ACCESSDENIED,"in archive mode");
    // }
    dlvInfo_->getGlossary().getTexts( texts );
}


void Delivery::setGlossary( const std::vector< std::string >& texts )
{
    if (!source_) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive/emergency mode");
    }
    dlvInfo_->getGlossary().setTexts( texts );
}

}
}
