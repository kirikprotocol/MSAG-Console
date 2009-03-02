#ifndef _SCAG_PVSS_BASE_BATCHREQUESTCOMPONENT_H
#define _SCAG_PVSS_BASE_BATCHREQUESTCOMPONENT_H

#include "AbstractCommand.h"

namespace scag2 {
namespace pvss {

class BatchRequestComponent : public AbstractCommand
{
public:
    BatchRequestComponent() : AbstractCommand() {}
    BatchRequestComponent( uint32_t seqNum ) : AbstractCommand(seqNum) {}

    // overriding the return type
    virtual BatchRequestComponent* clone() const = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_BATCHREQUESTCOMPONENT_H */
