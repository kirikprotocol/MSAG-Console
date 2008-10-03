#ifndef _SCAG_SESSIONS_EXTERNALTRANSACTION_H
#define _SCAG_SESSIONS_EXTERNALTRANSACTION_H

#include "util/debug.h"
#include "scag/util/storage/Serializer.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction;

class ExternalTransaction 
{
public:
    static ExternalTransaction* createAndDeserialize( util::storage::Deserializer& s ) 
        throw ( util::storage::DeserializerException );

public:
    virtual ~ExternalTransaction() {
        if ( active_ ) __trace__( "external transaction is still active!" );
    }
    // virtual std::string transactionId() const = 0;
    virtual void rollback() = 0;

    virtual void commit() = 0;

    virtual ExternalBillingTransaction* castToBilling() {
        return 0;
    }

    virtual util::storage::Serializer& serialize( util::storage::Serializer& s ) const = 0;
    virtual util::storage::Deserializer& deserialize( util::storage::Deserializer& s )
        throw ( util::storage::DeserializerException ) = 0;

protected:
    ExternalTransaction() : active_(true) {}

private:
    ExternalTransaction( const ExternalTransaction& );
    ExternalTransaction& operator = ( const ExternalTransaction& );

protected:
    bool active_;
};

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_EXTERNALTRANSACTION_H */
