#ifndef _SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H
#define _SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H

#include <cassert>
#include "scag/transport/CommandOperation.h"
#include "scag/re/base/RuleStatus2.h"

namespace scag2 {

namespace sessions {
class Session;
}

namespace transport {
namespace smpp {

class SmppCommand;

/// NOTE: this class does some side-effects on SmppCommand!
struct SmppOperationMaker
{
public:
    SmppOperationMaker( SmppCommand& thecmd,
                        sessions::Session& thesession );

    ~SmppOperationMaker() {
        assert( postproc_ );
    }

    /// method determine the type of operation and
    /// then create/obtain operation of this type from session.
    /// NOTE: side-effect: it also modifies command and session.
    /// at return st.status is set to STATUS_OK or STATUS_FAILED.
    void setupOperation( re::RuleStatus& st );

    /// post-process operation based on status.
    void postProcess( re::RuleStatus& st );

    /// the reason of failure in case of STATUS_FAILED.
    inline const char* what() const {
        return what_;
    }

    inline CommandOperation operationType() const {
        return optype_;
    }

private:
    SmppOperationMaker();

private:
    SmppCommand&       cmd;
    sessions::Session& session;
    CommandOperation   optype_;
    const char*        what_;
    bool               postproc_;
};

} // namespace smpp
} // namespace transport
} // namespace scag2

#endif /* !_SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H */
