#ifndef _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H
#define _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H

#include "ExternalTransaction.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction : public ExternalTransaction
{
public:
    static const uint32_t id = 1;

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

    virtual util::storage::Serializer& serialize( util::storage::Serializer& s ) const;
    virtual util::storage::Deserializer& deserialize( util::storage::Deserializer& s )
        throw ( util::storage::DeserializerException );


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
