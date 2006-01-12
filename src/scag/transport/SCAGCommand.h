#ifndef SCAG_TRANSPORT_COMMAND
#define SCAG_TRANSPORT_COMMAND

#include "util/int.h"

namespace scag { namespace transport
{
    enum TransportType
    {
        SMPP = 1, HTTP = 2, MMS = 3, WAP = 4
    };

    class SCAGCommand
    {
    public:

      virtual TransportType getType() const = 0;

      virtual int getRuleId() const = 0;
      virtual void setRuleId(int ruleId) = 0;

      virtual int64_t getOperationId() const = 0;
      virtual void setOperationId(int64_t op) = 0;

      virtual ~SCAGCommand() {};
    };

}}

#endif // SCAG_TRANSPORT_COMMAND
