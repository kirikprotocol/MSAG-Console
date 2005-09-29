#include "Rule.h"
#include "scag/transport/smpp/SmppCommand.h"
#include "scag/re/smpp/SmppEventHandler.h"

//#include "scag/SAX2Print.hpp"

namespace scag { namespace re 
{

using namespace scag::re::actions;
using namespace scag::transport::smpp;


Rule::~Rule()
{
    int key;
    EventHandler * value;

    for (IntHash <EventHandler *>::Iterator it = Handlers.First(); it.Next(key, value);)
    {
        delete value;
    }

    smsc_log_debug(logger,"Rule released");
}


RuleStatus Rule::process(SCAGCommand& command,Session& session)
{
    RuleStatus rs;
    if (command.getType() != transportType)
    {
        rs.result = false;
        rs.hasErrors = true;
        smsc_log_debug(logger,"Rule: command transport type and rule transport type are different");
        return rs;
    }

    smsc_log_debug(logger,"Process Rule...");

    int handlerType = ExtractHandlerType(command);
    if (!Handlers.Exist(handlerType)) 
    {
        rs.result = false;
        rs.hasErrors = true;
        smsc_log_debug(logger,"Rule: cannot find EventHandler for command");
        return rs;
    }

    EventHandler * eh = Handlers.Get(handlerType);
    try
    {
        rs = eh->process(command, session);
    } catch (Exception& e)
    {
        rs.result = false;
        rs.hasErrors = true;
        smsc_log_debug(logger,e.what());
        return rs;
    }
    return rs;
}

int Rule::ExtractHandlerType(SCAGCommand& command)
{
    SmppCommand * smpp = dynamic_cast<SmppCommand*>(&command);
    if (smpp) 
    {
        _SmppCommand * cmd = smpp->operator ->();
        if (!cmd) return 0;
        return cmd->get_commandId();
    }

    return 0;
}


EventHandler * Rule::CreateEventHandler()
{
    switch (transportType) 
    {
    case SMPP:
        return new SmppEventHandler();

    default:
        return 0;
    }
}





//////////////IParserHandler Interfase///////////////////////

IParserHandler * Rule::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    EventHandler * eh = 0;
    int nHId = 0;

    try
    {
        eh = CreateEventHandler();
        if (!eh) throw SCAGException("Rule: unknown RuleTransport to create EventHandler");

        nHId = eh->StrToHandlerId(name);
        PropertyObject propertyObject;
        propertyObject.HandlerId = nHId;
        propertyObject.transport = transportType;

        eh->init(params,propertyObject);
    } catch (SCAGException& e)
    {
        if (eh) delete eh;
        throw e;
    }
    if (Handlers.Exist(nHId)) 
    {
        delete eh;
        throw SCAGException("Rule: EventHandler with the same ID already exists");
    }

    Handlers.Insert(nHId,eh);
    return eh;
}

bool Rule::FinishXMLSubSection(const std::string& name)
{
    return (name.compare("rule") == 0);
}


void Rule::init(const SectionParams& params, PropertyObject propertyObject)
{
    if (!params.Exists("transport")) throw SCAGException("Rule: missing 'transport' parameter");

    std::string sTransport = params["transport"];

    if (sTransport == "SMPP") transportType = SMPP;
    else if (sTransport == "WAP") transportType = WAP;
    else if (sTransport == "MMS") transportType = MMS;
    else 
        throw SCAGException("Rule: invalid value '%s' for 'transport' parameter",sTransport.c_str());

    smsc_log_debug(logger,"Rule::Init");

}


//////////////IParserHandler Interfase///////////////////////

}}
