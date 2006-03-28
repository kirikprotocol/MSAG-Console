
#include "CommandBrige.h"
#include <scag/sessions/Session.h>

namespace scag { namespace re {


void CommandBrige::makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = handlerType;
        
    ev.Header.cProtocolId = 1;
    ev.Header.iServiceId = command.getServiceId();
    ev.Header.iServiceProviderId = 1;

    long now;
    time(&now);

    ev.Header.lDateTime = now;

    const char * str = getAbonentAddr(command).toString().c_str();
    sprintf((char *)ev.Header.pAbonentNumber,"%s",str);

    ev.Header.sCommandStatus = 1;
    ev.Header.sEventType = 1;

    ev.iOperatorId = 1;

    std::string unicodeSTR = getMessageBody(command);
    memcpy(ev.pMessageText, unicodeSTR.data(), unicodeSTR.size()); 

    sprintf((char *)ev.pSessionKey,"%s/%d", sessionPrimaryKey.abonentAddr.toString().c_str(),sessionPrimaryKey.BornMicrotime);
}


}}


