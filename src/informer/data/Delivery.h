#ifndef _INFORMER_DELIVERY_H
#define _INFORMER_DELIVERY_H

#include <memory>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "DeliveryInfo.h"
#include "ActivityLog.h"
#include "InputMessageSource.h"

namespace eyeline {
namespace informer {

class DeliveryImpl;

class Delivery
{
    friend class EmbedRefPtr< Delivery >;
    friend class EmbedRefPtr< DeliveryImpl >;
public:
    Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
              InputMessageSource*         source );

    virtual ~Delivery();

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }

    void updateDlvInfo( const DeliveryInfo& info );

    void addNewMessages( MsgIter begin, MsgIter end ) {
        source_->addNewMessages(begin,end);
    }

private:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        smsc_log_debug(log_,"D=%u ref=%u +1",dlvInfo_->getDlvId(),ref_);
        ++ref_;
    }
    void unref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        smsc_log_debug(log_,"D=%u ref=%u -1",dlvInfo_->getDlvId(),ref_);
        if (ref_<=1) {
            delete this;
        } else {
            --ref_;
        }
    }

protected:
    smsc::logger::Logger*                              log_;
    std::auto_ptr<DeliveryInfo>                        dlvInfo_;

    ActivityLog                                        activityLog_;
    InputMessageSource*                                source_;       // owned

    smsc::core::synchronization::Mutex                 lock_;
    unsigned                                           ref_;
};

typedef EmbedRefPtr< Delivery >  DeliveryPtr;

} // informer
} // smsc

#endif
