#ifndef SCAG_BILL_EWALLET_REQUESTVISITOR_H
#define SCAG_BILL_EWALLET_REQUESTVISITOR_H

namespace scag2 {
namespace bill {
namespace ewallet {

// class Ping;
// class Auth;
class Open;
class Commit;
class Rollback;
class Check;
class Transfer;
class Info;

class RequestVisitor
{
public:
    // virtual bool visitPing( Ping& ) = 0;
    // virtual bool visitAuth( Auth& ) = 0;
    virtual bool visitOpen( Open& ) = 0;
    virtual bool visitCommit( Commit& ) = 0;
    virtual bool visitRollback( Rollback& ) = 0;
    virtual bool visitCheck( Check& ) = 0;
    virtual bool visitTransfer( Transfer& ) = 0;
    virtual bool visitInfo( Info& ) = 0;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
