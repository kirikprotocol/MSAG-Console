#ifndef SCAG_TRANSPORT_COMMAND
#define SCAG_TRANSPORT_COMMAND

#include "util/int.h"
#include "sms/sms.h"

namespace scag { namespace transport
{
    const uint8_t T_TYPE_SMPP = 1;
    const uint8_t T_TYPE_WAP  = 2;
    const uint8_t T_TYPE_MMS  = 3;

    using smsc::sms::Address;

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

        virtual bool isFinalCommand() const = 0;
        virtual int getKey() const = 0;
        virtual Address getDestAddr() const = 0;
        virtual Address getAbonentAddr() const = 0;
        virtual int16_t getUMR() const = 0;
    };

}}

#endif // SCAG_TRANSPORT_COMMAND

