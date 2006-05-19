#ifndef SCAG_TRANSPORT_COMMAND
#define SCAG_TRANSPORT_COMMAND

#include "util/int.h"
#include <core/buffers/Hash.hpp>

namespace scag { namespace transport
{

using smsc::core::buffers::Hash;

    enum TransportType
    {
        SMPP = 1, HTTP = 2, MMS = 3
    };


	    
    
    class SCAGCommand
    {
    public:

      //static Hash<TransportType> TransportTypeHash;
      //static Hash<TransportType> InitTransportTypeHash();
      
      virtual TransportType getType() const = 0;

      virtual int getServiceId() const = 0;
      virtual void setServiceId(int serviceId) = 0;

      virtual int64_t getOperationId() const = 0;
      virtual void setOperationId(int64_t op) = 0;
      virtual uint8_t getCommandId() const = 0;

      virtual ~SCAGCommand() {};
    };

}}

#endif // SCAG_TRANSPORT_COMMAND
