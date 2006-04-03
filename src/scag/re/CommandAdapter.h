#ifndef _SCAG_COMMAND_ADAPTER_
#define _SCAG_COMMAND_ADAPTER_


#include <scag/transport/SCAGCommand.h>
#include "scag/re/smpp/SmppAdapter.h"

#include <string>


namespace scag { namespace re { 

using namespace scag::transport;
using scag::re::smpp::SmppCommandAdapter;

enum AccessType 
{
    atNoAccess = 0,
    atRead  = 1,
    atWrite = 2,
    atReadWrite = 3
};


class CommandAdapter
{

public:
    static AccessType CheckAccess(int handlerType,const std::string& PropertyName,TransportType transportType)
    {
        switch (transportType) 
        {

        case SMPP:
            return SmppCommandAdapter::CheckAccess(handlerType,PropertyName);

        }
        return atNoAccess;
    }  
};



}}

#endif
