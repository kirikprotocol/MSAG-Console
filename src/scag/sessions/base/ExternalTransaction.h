#ifndef _SCAG_SESSIONS_EXTERNALTRANSACTION_H
#define _SCAG_SESSIONS_EXTERNALTRANSACTION_H

#include "util/debug.h"
#include "scag/util/io/Serializer.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction;

class ExternalTransaction 
{
public:
    static ExternalTransaction* createAndDeserialize( util::io::Deserializer& s ) 
        /* throw ( util::io::DeserializerException ) */;

public:
    virtual ~ExternalTransaction() {
        if ( active_ ) __trace__( "external transaction is still active!" );
    }
    // virtual std::string transactionId() const = 0;
    virtual void rollback( bool timeout ) = 0;

    virtual void commit() = 0;

    virtual ExternalBillingTransaction* castToBilling() {
        return 0;
    }

    virtual util::io::Serializer& serialize( util::io::Serializer& s ) const = 0;
    virtual util::io::Deserializer& deserialize( util::io::Deserializer& s )
        /* throw ( util::io::DeserializerException ) */ = 0;

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
