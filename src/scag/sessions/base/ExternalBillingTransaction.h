#ifndef _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H
#define _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H

#include "ExternalTransaction.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction : public ExternalTransaction
{
public:
    typedef bill::billid_type billid_type;

    static const uint32_t id = 1; // is used to deserialize back

public:
    explicit ExternalBillingTransaction( billid_type billid ) :
    billid_(billid) {}

    virtual ~ExternalBillingTransaction() {
        // delete keywords_;
    }

    virtual void commit() {
        bill::BillingManager::Instance().Commit( billid_ );
        active_ = false;
    }
    virtual void rollback( bool timeout ) {
        bill::BillingManager::Instance().Rollback( billid_, timeout );
        active_ = false;
    }

    virtual ExternalBillingTransaction* castToBilling() {
        return this;
    }

    virtual util::io::Serializer& serialize( util::io::Serializer& s ) const;
    virtual util::io::Deserializer& deserialize( util::io::Deserializer& s )
        /* throw ( util::io::DeserializerException ) */;


    virtual void info( bill::BillingInfoStruct& bis,
                       bill::TariffRec& tr ) const 
    {
        bill::BillingManager::Instance().Info( billid_, bis, tr );
    }

    billid_type billId() const {
        return billid_;
    }

    /*
    inline const std::string* getKeywords() const {
        return keywords_;
    }

    void setKeywords( const std::string& kw ) {
        if ( keywords_ ) delete keywords_;
        if ( kw.empty() ) keywords_ = 0;
        else keywords_ = new std::string(kw);
    }
     */

private:
    billid_type  billid_;
    // NOTE: we don't need keywords anymore
    // std::string* keywords_;
};

}
}

#endif /* ! _SCAG_SESSIONS_BASE_EXTERNALBILLINGTRANSACTION_H */
