#ifndef SCAG_TRANSPORT_SMPP_COMMAND
#define SCAG_TRANSPORT_SMPP_COMMAND

#include <scag/transport/SCAGCommand.h>

namespace scag { namespace transport { namespace smpp 
{

    class SmppCommand : public SCAGCommand
    {
    public:

        SmppCommand() : SCAGCommand(TransportType:SMPP) {};
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_COMMAND

