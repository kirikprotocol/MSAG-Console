#ifndef _SCAG_COMMAND_ADAPTER2_
#define _SCAG_COMMAND_ADAPTER2_

#include "scag/transport/SCAGCommand2.h"
#include "SmppAdapter2.h"
#include "HttpAdapter2.h"
#include "SessionAdapter2.h"

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
            // FIXME: http
            fprintf( stderr, "NOT IMPL YET\n");
            ::abort();
            // return HttpCommandAdapter::CheckAccess(handlerType,PropertyName);
        default:
            break;
        }
        return atNoAccess;
    }  
};

}}

#endif
