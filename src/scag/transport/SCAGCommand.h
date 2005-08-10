#ifndef SCAG_TRANSPORT_COMMAND
#define SCAG_TRANSPORT_COMMAND

#include "util/int.h"

namespace scag { namespace transport
{
    
    const uint8_t T_TYPE_SMPP = 1;
    const uint8_t T_TYPE_WAP  = 2;
    const uint8_t T_TYPE_MMS  = 3;
                           
    enum TransportType
    {
        SMPP = T_TYPE_SMPP,
        WAP  = T_TYPE_WAP,
        MMS  = T_TYPE_MMS
    }; 

    class SCAGCommand
    {
    protected:

        TransportType type;
        SCAGCommand(TransportType _type) : type(_type) {};
    public:

        virtual ~SCAGCommand() {};
        inline TransportType getType() const {
            return type;
        };
    };

}}

#endif // SCAG_TRANSPORT_COMMAND

