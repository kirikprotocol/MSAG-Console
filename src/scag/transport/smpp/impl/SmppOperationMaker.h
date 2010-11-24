#ifndef _SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H
#define _SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H

#include "scag/transport/CommandOperation.h"
#include "scag/re/base/RuleStatus2.h"
#include "scag/re/base/ActionContext2.h"

namespace scag2 {

namespace sessions {
class Session;
class ActiveSession;
}

namespace util {
class HRTiming;
}


namespace transport {
namespace smpp {

class SmppCommand;

/// NOTE: this class does some side-effects on SmppCommand!
struct SmppOperationMaker
{
public:
    class SarRegistry;

    SmppOperationMaker( const char*                 where,
                        std::auto_ptr<SmppCommand>& thecmd,
                        sessions::ActiveSession&    thesession,
                        smsc::logger::Logger*       logger );

    ~SmppOperationMaker() {
        // NOTE: postproc_ may be not set, in case of failure in setupOperation()
    }

    /// process command/session:
    /// 1. analyse the command, determine the type of operation;
    /// 1a. create/fetch the session (already done);
    /// 2. create/fetch the operation;
    /// 3. preprocess command/operation before RE execution;
    /// 4. invoke RE;
    /// 5. postprocess command/operation.
    void process( re::RuleStatus& st, scag2::re::actions::CommandProperty& cp, util::HRTiming* hrt = 0 );

    /// the reason of failure in case of STATUS_FAILED.
    inline const char* what() const {
        return what_;
    }

    inline CommandOperation operationType() const {
        return optype_;
    }

private:
    /// method determine the type of operation and
    /// then create/obtain operation of this type from session.
    /// NOTE: side-effect: it also modifies command and session.
    /// at return st.status is set to STATUS_OK or STATUS_FAILED.
    void setupOperation( re::RuleStatus& st,
                         re::actions::CommandProperty& cp );

    /// post-process operation based on status.
    void postProcess( re::RuleStatus& st,
                      re::actions::CommandProperty& cp );

    SmppOperationMaker();

    inline void fail( const char* msg,
                      re::RuleStatus& st,
                      int reason ) {
        what_ = msg;
        st.status = re::STATUS_FAILED;
        st.result = reason;
        smsc_log_warn( log_, "%s: failure: %s, res=%d", where_, what_, reason );
    }

private:
    const char*                  where_;
    std::auto_ptr<SmppCommand>&  cmd_;
    sessions::ActiveSession&     session_;
    CommandOperation             optype_;
    const char*                  what_;
    bool                         postproc_;
    time_t                       currentTime_;
    smsc::logger::Logger*        log_;
    int                          currentIndex_; // parameters of the sms
    int                          lastIndex_;    // parameters of the sms
    int                          sarmr_;
};

} // namespace smpp
} // namespace transport
} // namespace scag2

#endif /* !_SCAG_TRANSPORT_SMPP_SMPPOPERATIONMAKER_H */
