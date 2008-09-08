#ifndef _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H
#define _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H

#include "ExternalTransaction.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction : public ExternalTransaction
{
public:
    explicit ExternalBillingTransaction( int billid ) :
    billid_(billid) {}

    virtual void commit() {
        bill::BillingManager::Instance().Commit( billid_ );
        active_ = false;
    }
    virtual void rollback() {
        bill::BillingManager::Instance().Rollback( billid_ );
        active_ = false;
    }

    virtual ExternalBillingTransaction* castToBilling() {
        return this;
    }

    virtual void info( bill::BillingInfoStruct& bis,
                       bill::TariffRec& tr ) const 
    {
        bill::BillingManager::Instance().Info( billid_, bis, tr );
    }

    int billId() const {
        return billid_;
    }

private:
    int billid_;
};

}
}

#endif /* ! _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H */
