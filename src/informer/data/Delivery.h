#ifndef _INFORMER_DELIVERY_H
#define _INFORMER_DELIVERY_H

#include <memory>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "DeliveryInfo.h"
#include "InputMessageSource.h"

namespace eyeline {
namespace informer {

class UserInfo;
class ActivityLog;

class Delivery
{
    friend class EmbedRefPtr< Delivery >;
protected:
    Delivery( DeliveryInfo*               dlvInfo,
              InputMessageSource*         source );

public:
    virtual ~Delivery();

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }
    inline const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }
    inline const UserInfo& getUserInfo() const {
        return dlvInfo_->getUserInfo();
    }

    virtual void updateDlvInfo( const DeliveryInfoData& data ) = 0;

    inline DlvState getState( msgtime_type* planTime = 0 ) const {
        if (planTime) *planTime = planTime_;
        return state_;
    }

    // change the state of the delivery
    virtual void setState( DlvState state, msgtime_type planTime = 0 ) = 0;
    virtual void getRegionList( std::vector< regionid_type >& regIds ) const = 0;

    void addNewMessages( MsgIter begin, MsgIter end );
    void dropMessages( const std::vector< msgid_type >& msgids );

    void getGlossary( std::vector< std::string >& texts ) const;
    void setGlossary( const std::vector< std::string >& texts );

protected:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        // smsc_log_debug(log_,"D=%u ref=%u +1",getDlvId(),ref_);
        ++ref_;
    }
    void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(lock_);
            // smsc_log_debug(log_,"D=%u ref=%u -1",getDlvId(),ref_);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }

protected:
    smsc::logger::Logger*                              log_;

    smsc::core::synchronization::Mutex                 stateLock_;
    DlvState                                           state_;
    msgtime_type                                       planTime_;

    std::auto_ptr<DeliveryInfo>                        dlvInfo_;
    ActivityLog*                                       activityLog_;
    InputMessageSource*                                source_;       // owned

    smsc::core::synchronization::Mutex                 lock_;
    unsigned                                           ref_;

};

typedef EmbedRefPtr< Delivery >  DeliveryPtr;

} // informer
} // smsc

#endif
