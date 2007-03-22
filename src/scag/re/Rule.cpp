#include "Rule.h"
#include "scag/transport/smpp/SmppCommand.h"
#include "scag/transport/http/HttpCommand.h"
#include "scag/re/smpp/SmppEventHandler.h"
#include "scag/re/http/HttpEventHandler.h"
#include "RuleEngine.h"


namespace scag { namespace re 
{

using namespace scag::re::actions;
using namespace scag::transport::smpp;
using namespace scag::transport::http;
using namespace scag::re::http;

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
        rs.status = STATUS_FAILED;
        throw SCAGException("Rule: command transport type and rule transport type are different");
        //smsc_log_error(logger,"Rule: command transport type and rule transport type are different");
        //return rs;
    }

    smsc_log_debug(logger,"Process Rule... (%d Event Handlers registered)", Handlers.Count());

    EventHandlerType handlerType = CommandBrige::getHandlerType(command);

    smsc_log_debug(logger,"Event Handlers found. (id=%d)", handlerType);

    if(!Handlers.Exist(handlerType)) 
    {
        rs.status = STATUS_FAILED;
        smsc_log_warn(logger,"Rule: cannot find EventHandler for command");
        return rs;
    }

    EventHandler * eh = Handlers.Get(handlerType);
    try
    {
        rs = eh->process(command, session);
    }
    catch (Exception& e)
    {
        rs.status = STATUS_FAILED;
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
        return rs;
    } 
    catch (std::exception& e)
    {
        rs.status = STATUS_FAILED;
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
    //abort();
        return rs;
    } 
    catch (...)
    {
        rs.status = STATUS_FAILED;
        smsc_log_error(logger,"EH Rule top level exception: Unknown system error");
        return rs;
    }
    return rs;
}

EventHandler * Rule::CreateEventHandler()
{
    switch (transportType) 
    {
    case SMPP:
        return new SmppEventHandler();

    case HTTP:
        return new HttpEventHandler();

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
        propertyObject.transport = transportType;
        propertyObject.HandlerId = nHId;

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

    TransportType * transportPTR = RuleEngine::Instance().getTransportTypeHash().GetPtr(sTransport.c_str());
    
    if (!transportPTR) throw SCAGException("Rule: invalid value '%s' for 'transport' parameter",sTransport.c_str());

    transportType = *transportPTR;

    smsc_log_debug(logger,"Rule::Init");
}


//////////////IParserHandler Interfase///////////////////////

}}
