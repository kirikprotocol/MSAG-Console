#ifndef SCAG_BILL_EWALLET_RESPONSEVISITOR_H
#define SCAG_BILL_EWALLET_RESPONSEVISITOR_H

namespace scag2 {
namespace bill {
namespace ewallet {

// class PingResp;
// class AuthResp;
class OpenResp;
class CommitResp;
class RollbackResp;
class CheckResp;

class ResponseVisitor
{
public:
    // virtual bool visitPingResp( PingResp& ) = 0;
    // virtual bool visitAuthResp( AuthResp& ) = 0;
    virtual bool visitOpenResp( OpenResp& ) = 0;
    virtual bool visitCommitResp( CommitResp& ) = 0;
    virtual bool visitRollbackResp( RollbackResp& ) = 0;
    virtual bool visitCheckResp( CheckResp& ) = 0;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
