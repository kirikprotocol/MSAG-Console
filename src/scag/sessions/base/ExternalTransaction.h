#ifndef _SCAG_SESSIONS_EXTERNALTRANSACTION_H
#define _SCAG_SESSIONS_EXTERNALTRANSACTION_H

#include "util/debug.h"

namespace scag2 {
namespace sessions {

class ExternalBillingTransaction;

class ExternalTransaction 
{
public:
    virtual ~ExternalTransaction() {
        if ( active_ ) __trace__( "external transaction is still active!" );
    }
    // virtual std::string transactionId() const = 0;
    virtual void rollback() = 0;

    /// FIXME: or should it return bool?
    virtual void commit() = 0;

    virtual ExternalBillingTransaction* castToBilling() {
        return 0;
    }

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
