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

    //smsc_log_debug(logger,"Rule released");
}


RuleStatus Rule::process(SCAGCommand& command,Session& session)
{
    //TODO: fill rs fields

    RuleStatus rs;
    if (command.getType() != transportType)
    {
        rs.status = false;
        throw SCAGException("Rule: command transport type and rule transport type are different");
        //smsc_log_error(logger,"Rule: command transport type and rule transport type are different");
        //return rs;
    }

    smsc_log_debug(logger,"Process Rule...");

    EventHandlerType handlerType = CommandBrige::getHandlerType(command);

    if (!Handlers.Exist(handlerType)) 
    {
        rs.status = false;
        smsc_log_warn(logger,"Rule: cannot find EventHandler for command");
        return rs;
    }

    EventHandler * eh = Handlers.Get(handlerType);
    try
    {
        rs = eh->process(command, session);
    } catch (Exception& e)
    {
        rs.status = false;
        smsc_log_error(logger,e.what());
        return rs;
    } catch (...)
    {
        rs.status = false;
        smsc_log_error(logger,"EvendHandler: Process terminated - Unknown system error");
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

    std::wstring sTransport = params["transport"];

    //std:: cout << "^^^^^^^^^^^^" << ConvertWStrToStr(sTransport.c_str()) << "^^" << sTransport.size() << std::endl;

    if (sTransport == ConvertStrToWStr("SMPP")) transportType = SMPP;
    else if (sTransport == ConvertStrToWStr("WAP")) transportType = WAP;
    else if (sTransport == ConvertStrToWStr("MMS")) transportType = MMS;
    else 
        throw SCAGException("Rule: invalid value '%s' for 'transport' parameter",ConvertWStrToStr(sTransport).c_str());

    smsc_log_debug(logger,"Rule::Init");

}


//////////////IParserHandler Interfase///////////////////////

}}
