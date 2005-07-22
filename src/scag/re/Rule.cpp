#include "Rule.h"

namespace scag { namespace re 
{

using smsc::core::buffers::IntHash;
using namespace scag::re::actions;


/////////////////////////////ActionFactory/////////////////////////

Action * ActionFactory::CreateAction(const std::string& name) const
{
    if (name=="set") return new ActionSet();
    if (name=="if")  return new ActionIf();
    if (name=="return") return new ActionReturn();

    return 0;
}

void ActionFactory::FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const
{
    TagHash["set"] = tgActionSection;
    TagHash["if"] = tgActionSection;
    TagHash["return"] = tgActionSection;
}


/////////////////////////////Rule///////////////////////////////////


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

    /*
    int key;
    for (IntHash<EventHandler *>::Iterator it = Handlers.First(); it.Next(key, eh);)
    {
        
        rs = eh->process(command);
        break;
    } */

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


//////////////IParserHandler Interfase///////////////////////

void Rule::SetChildObject(IParserHandler * child)
{
    if (!child) return;

    EventHandler * eh = dynamic_cast<EventHandler *>(child);
    if (!eh) return throw Exception("Rule: unrecognized child object");

    if (Handlers.Exist(eh->GetHandlerType())) throw Exception("Rule: EventHandler with the same type already exists");
    Handlers.Insert(eh->GetHandlerType(),eh);
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
}


//////////////IParserHandler Interfase///////////////////////

}}
