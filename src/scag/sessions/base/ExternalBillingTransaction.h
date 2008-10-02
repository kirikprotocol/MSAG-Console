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
    billid_(billid), keywords_(0) {}

    virtual ~ExternalBillingTransaction() {
        delete keywords_;
    }

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

    const std::string& getKeywords() const {
        static const std::string empty;
        return keywords_ ? *keywords_ : empty;
    }

    void setKeywords( const std::string& kw ) {
        if ( keywords_ ) delete keywords_;
        if ( kw.empty() ) keywords_ = 0;
        else keywords_ = new std::string(kw);
    }

private:
    int          billid_;
    std::string* keywords_;
};

}
}

#endif /* ! _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H */
