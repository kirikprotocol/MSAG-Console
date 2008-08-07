#ifndef _SCAG_COMMAND_ADAPTER2_
#define _SCAG_COMMAND_ADAPTER2_


#include "scag/transport/SCAGCommand2.h"
#include "scag/re/smpp/SmppAdapter2.h"
#include "scag/re/http/HttpAdapter2.h"
#include "scag/re/session/SessionAdapter2.h"

#include <string>

namespace scag2 {
namespace re { 

using namespace transport;
using re::smpp::SmppCommandAdapter;
using re::http::HttpCommandAdapter;
using re::session::SessionAdapter;

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
        default:
            break;
        }
        return atNoAccess;
    }  
};

}}

#endif
