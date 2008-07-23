#ifndef _SCAG_SESSIONS_EXTERNALTRANSACTION_H
#define _SCAG_SESSIONS_EXTERNALTRANSACTION_H

namespace scag {
namespace sessions2 {

    class ExternalTransaction 
    {
    public:
        virtual ~ExternalTransaction() {}
        // virtual std::string transactionId() const = 0;
        virtual void rollback() = 0;
        virtual void commit() = 0;
    };

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_EXTERNALTRANSACTION_H */
