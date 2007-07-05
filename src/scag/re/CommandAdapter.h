#ifndef _SCAG_COMMAND_ADAPTER_
#define _SCAG_COMMAND_ADAPTER_


#include <scag/transport/SCAGCommand.h>
#include "scag/re/smpp/SmppAdapter.h"
#include "scag/re/http/HttpAdapter.h"
#include "scag/re/session/SessionAdapter.h"

#include <string>


namespace scag { namespace re { 

using namespace scag::transport;
using scag::re::smpp::SmppCommandAdapter;
using scag::re::http::HttpCommandAdapter;
using scag::re::session::SessionAdapter;

class CommandAdapter
{

public:
    static AccessType CheckAccess(int handlerType,const std::string& PropertyName,TransportType transportType)
    {
	if(handlerType == EH_SESSION_DESTROY)
	    return SessionAdapter::CheckAccess(handlerType, PropertyName);
        switch (transportType) 
        {
            case SMPP:
	        return SmppCommandAdapter::CheckAccess(handlerType,PropertyName);
            case HTTP:
	        return HttpCommandAdapter::CheckAccess(handlerType,PropertyName);
        }
        return atNoAccess;
    }  
};



}}

#endif
