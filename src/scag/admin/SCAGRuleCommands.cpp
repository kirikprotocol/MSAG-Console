#include "SCAGRuleCommands.h"
#include "util/xml/utilFunctions.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag { namespace admin {

using namespace smsc::util::xml;
using namespace scag::exceptions;


CommandRuleBase::CommandRuleBase(const CommandIds::IDS ID, const xercesc::DOMDocument * doc) 
    : AdminCommand((Command::Id)ID, doc)

{
    switch (ID) 
    {
    case CommandIds::addRule:
        m_ProcessName = "add";
        break;
    case CommandIds::updateRule:
        m_ProcessName = "update";
        break;
    case CommandIds::removeRule:
        m_ProcessName = "remove";
        break;
    }
}


void CommandRuleBase::init()
{
    smsc_log_info(logger, "SCAGCommand '%s' rule got parameters:", m_ProcessName.c_str());
    
    BEGIN_SCAN_PARAMS
    GETINTPARAM(key.serviceId, "serviceId")
    GETSTRPARAM_(strTransport, "transport")
    END_SCAN_PARAMS

    scag::transport::TransportType * ttype;
    ttype = scag::transport::SCAGCommand::TransportTypeHash.GetPtr(strTransport.c_str());

  /*  if (scag::transport::SCAGCommand::TransportTypeHash.GetCount() == 0) 
    {
        smsc_log_error(logger,"Achtung!!! :))");
    }

    char * key = 0;
    scag::transport::TransportType value;

    scag::transport::SCAGCommand::TransportTypeHash.First();
    for (Hash <scag::transport::TransportType>::Iterator it = scag::transport::SCAGCommand::TransportTypeHash.getIterator(); it.Next(key, value);)
    {
        smsc_log_error(logger,"%d %s",value, key);
    }   */


    
    if (!ttype) 
    {
        smsc_log_error(logger,"Unknown transport parameter '%s'", strTransport.c_str());
        throw AdminException("Unknown transport parameter '%s'", strTransport.c_str());
    }  

    key.transport = *ttype;

    if (key.serviceId == -1) 
    {
        smsc_log_error(logger,"Missing serviceId parameter");
        throw AdminException("Missing serviceId parameter");
    }

}

Response * CommandRuleBase::CreateResponse(scag::Scag * SmscApp)
{
   /* if (hasErrors) 
    {
        char msg[1024];                                         
        sprintf(msg, "SCAGCommand '%s' rule: cannot process command - parameters is invalid", m_ProcessName.c_str());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    }        */

    try {

        processRuleCommand();

    } catch (RuleEngineException& e){                             
        char desc[512];                                         
        sprintf(desc, "Failed to %s rule (%s transport). RuleEngineException exception: %s. Error in rule_%d.xml in line %d.", m_ProcessName.c_str(), strTransport.c_str(), e.what(), key.serviceId, e.getLineNumber());
        Variant res(smsc::admin::service::StringListType); 

        res.appendValueToStringList(desc);
        smsc_log_info(logger, desc);                            
        char buff[32];
        sprintf(buff,"%d",e.getLineNumber());			
        res.appendValueToStringList(buff);			
        return new Response(Response::Ok, res);

    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to %s rule. Details: %s", m_ProcessName.c_str(), e.what());
        smsc_log_error(logger, msg);                            
        return new Response(Response::Error, msg);

    } catch (...) {
        smsc_log_warn(logger, "Failed to %s rule. Unknown exception", m_ProcessName.c_str());
        throw AdminException("Failed to %s rule. Unknown exception", m_ProcessName.c_str());
    }

    Variant okRes("");
    return new Response(Response::Ok, okRes);
}


//================================================================
//================ Rule commands =================================

using smsc::admin::service::Variant;
using scag::exceptions::RuleEngineException;

CommandAddRule::CommandAddRule(const xercesc::DOMDocument * document)
    : CommandRuleBase(CommandIds::addRule, document)
{


}

void CommandAddRule::processRuleCommand()
{
    smsc_log_info(logger, "CommandAddRule is processing...");

    scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
    re.updateRule(key);

    smsc_log_info(logger, "CommandAddRule is processed ok");
}

//================================================================

CommandRemoveRule::CommandRemoveRule(const xercesc::DOMDocument * document)
    : CommandRuleBase(CommandIds::removeRule, document)
{

}

void CommandRemoveRule::processRuleCommand()
{
    smsc_log_info(logger, "CommandRemoveRule is processing...");

    scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
    re.removeRule(key);

    smsc_log_info(logger, "CommandRemoveRule is processed ok");
}

//================================================================

CommandUpdateRule::CommandUpdateRule(const xercesc::DOMDocument * document)
    : CommandRuleBase(CommandIds::updateRule, document)
{
}

void CommandUpdateRule::processRuleCommand()
{
    smsc_log_info(logger, "CommandUpdateRule is processing...");

    scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
    re.updateRule(key);

    smsc_log_info(logger, "CommandUpdateRule is processed ok.");
}






}}
