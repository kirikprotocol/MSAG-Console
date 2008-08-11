#ifndef _SCAG_RE_SMPP_SMPPTRANSPORTRULE_H
#define _SCAG_RE_SMPP_SMPPTRANSPORTRULE_H

#include "scag/re/TransportRule.h"

namespace scag2 {

namespace transport {
namespace smpp {
class SmppCommand;
}
}

namespace re {

class CSmppDescriptor;

namespace smpp {

class SmppTransportRule : public TransportRule
{
public:
    static void ProcessModifyRespCommandOperation( sessions::Session& session,
                                                   transport::smpp::SmppCommand& command,
                                                   CSmppDescriptor& smppDescriptor );
    static void ProcessModifyCommandOperation( sessions::Session& session,
                                               transport::smpp::SmppCommand& command,
                                               CSmppDescriptor& smppDescriptor );
    static void ModifyOperationAfterExecuting( sessions::Session& session,
                                               transport::smpp::SmppCommand& command,
                                               RuleStatus& status,
                                               CSmppDescriptor& smppDescriptor );

public:
    virtual ~SmppTransportRule() {}
    virtual void setupSessionOperation( transport::SCAGCommand& command,
                                        sessions::Session& session,
                                        RuleStatus& rs );
    virtual void resetSessionOperation( transport::SCAGCommand& command,
                                        sessions::Session& session,
                                        RuleStatus& rs );
};

} // namespace smpp
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_SMPP_SMPPTRANSPORTRULE_H */
