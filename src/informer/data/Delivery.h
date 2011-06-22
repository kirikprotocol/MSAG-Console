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

    /// get the state of the delivery and optional planTime
    /// planTime is used only for DLVSTATE_PLANNED.
    /// if it is 0 than start delivery at startDate or now (what is later).
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

    msgtime_type getLocalStartDateInUTC() const;

protected:
    virtual timediff_type getMaximalRegionalOffset() const = 0;

    void ref() {
        smsc::core::synchronization::MutexGuard mg(reflock_);
        // smsc_log_debug(log_,"D=%u ref=%u +1",getDlvId(),ref_);
        ++ref_;
    }
    void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
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

    smsc::core::synchronization::Mutex                 reflock_;
    unsigned                                           ref_;
};


class DeliveryFilter
{
public:
    virtual ~DeliveryFilter() {}
    virtual bool filter( const Delivery& d ) = 0;
};


typedef EmbedRefPtr< Delivery >  DeliveryPtr;

typedef std::vector< DeliveryPtr > DeliveryList;

} // informer
} // smsc

#endif
