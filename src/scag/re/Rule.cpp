#include "Rule.h"
#include "scag/transport/smpp/SmppCommand.h"
#include "SAX2Print.hpp"

#include "SmppEventHandler.h"

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


RuleStatus Rule::process(SCAGCommand& command)
{
    RuleStatus rs;
    EventHandler * eh;

    if (command.getType() != transportType) 
        throw Exception("Rule: command transport type and rule transport type are different");

    smsc_log_debug(logger,"Process Rule...");

    HandlerType handlerType = ExtractHandlerType(command);
    if (!Handlers.Exist(handlerType))  throw Exception("Rule: cannot find EventHandler for command");

    eh = Handlers.Get(handlerType);
    rs = eh->process(command);

    return rs;
}

HandlerType Rule::ExtractHandlerType(SCAGCommand& command)
{
    SmppCommand * smpp = dynamic_cast<SmppCommand*>(&command);
    if (smpp) 
    {
        return htSubmit;
        /*
        
        */
    }

    return htUnknown;
}


EventHandler * Rule::CreateEventHandler()
{
    switch (transportType) 
    {
    case SMPP:
        return new SmppEventHandler();
        break;

    default:
        return 0;
        break;
    }
}


//////////////IParserHandler Interfase///////////////////////

IParserHandler * Rule::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    EventHandler * eh = 0;

    if (name.compare("handler") == 0) 
    {
        try
        {
            eh = CreateEventHandler();
            if (!eh) throw Exception("Rule: unknown RuleTransport to create EventHandler");

            eh->init(params);
        } catch (Exception& e)
        {
            if (eh) delete eh;
            throw e;
        }

        if (Handlers.Exist(eh->GetHandlerType())) 
        {
            delete eh;
            throw Exception("Rule: EventHandler with the same type already exists");
        }

        Handlers.Insert(eh->GetHandlerType(),eh);
        return eh;
    }
    else
    {
        std::string msg("Rule: unrecognized child object '");
        msg.append(name);
        msg.append("' to create");
        throw Exception(msg.c_str());
    }
}

bool Rule::FinishXMLSubSection(const std::string& name)
{
    return (name.compare("rule") == 0);
}


void Rule::init(const SectionParams& params)
{
    if (!params.Exists("name")) throw Exception("Rule: missing 'name' parameter");
    if (!params.Exists("transport")) throw Exception("Rule: missing 'transport' parameter");

    std::string sTransport = params["transport"];

    if (sTransport.compare("SMPP")==0) transportType = SMPP;
    else if (sTransport.compare("WAP")==0) transportType = WAP;
         else if (sTransport.compare("MMS")==0) transportType = MMS;
              else 
              {
                  std::string msg("Rule: invalid value '");
                  msg.append(sTransport);
                  msg.append("' for 'transport' parameter");
                  throw Exception(msg.c_str());
              }

    smsc_log_debug(logger,"Rule::Init");

}


//////////////IParserHandler Interfase///////////////////////

}}
