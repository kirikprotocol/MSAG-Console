#include "Rule.h"
#include "scag/transport/smpp/SmppCommand.h"
#include "scag/transport/http/HttpCommand.h"
#include "scag/re/smpp/SmppEventHandler.h"
#include "scag/re/http/HttpEventHandler.h"
#include "scag/re/session/SessionEventHandler.h"
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
        delete value;

    smsc_log_debug(logger,"Rule released");
}


void Rule::process(SCAGCommand& command,Session& session, RuleStatus& rs)
{
    smsc_log_debug(logger,"Process Rule... (%d Event Handlers registered)", Handlers.Count());

    if(session.isNew())
    {
        processSession(session, rs);
        if(rs.status != STATUS_OK)
           return;
    }

    EventHandlerType handlerType = CommandBrige::getHandlerType(command);

    smsc_log_debug(logger,"Event Handlers found. (id=%d)", handlerType);

    try
    {
        if(Handlers.Exist(handlerType)) 
        {
            EventHandler * eh = Handlers.Get(handlerType);
            eh->process(command, session, rs);
            if(rs.status == STATUS_OK)
                session.getLongCallContext().continueExec = false;
            return;
        }
        smsc_log_warn(logger,"Rule: cannot find EventHandler for command");
    }
    catch (Exception& e)
    {
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
    } 
    catch (std::exception& e)
    {
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
    } 
    catch (...)
    {
        smsc_log_error(logger,"EH Rule top level exception: Unknown system error");
    }
    rs.status = STATUS_FAILED;
}

void Rule::processSession(Session& session, RuleStatus& rs)
{
    smsc_log_debug(logger,"Process session rule... (%d Event Handlers registered)", Handlers.Count());

    try
    {
        if(Handlers.Exist(session.isNew() ? EH_SESSION_INIT : EH_SESSION_DESTROY))
        {
            SessionEventHandler* eh = (SessionEventHandler*)Handlers.Get(session.isNew() ? EH_SESSION_INIT : EH_SESSION_DESTROY);
            eh->_process(session, rs);
            if(rs.status == STATUS_OK)
                session.getLongCallContext().continueExec = false;
        }
        if(session.isNew()) session.setNew(false);        
//        smsc_log_warn(logger,"session rule: cannot find EventHandler for command");
        return;
    }
    catch (Exception& e)
    {
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
    } 
    catch (std::exception& e)
    {
        smsc_log_error(logger, "EH Rule top level exception: %s", e.what());
    } 
    catch (...)
    {
        smsc_log_error(logger,"EH Rule top level exception: Unknown system error");
    }
    rs.status = STATUS_FAILED;
}

EventHandler * Rule::CreateEventHandler()
{
    switch (transportType) 
    {
    case SMPP:
        return new SmppEventHandler();
    case HTTP:
        return new HttpEventHandler();
    }
    throw SCAGException("Rule: unknown RuleTransport to create EventHandler");
}


//////////////IParserHandler Interfase///////////////////////

IParserHandler * Rule::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    EventHandler * eh = 0;
    int nHId = 0;
    bool i;

    try
    {
        if( (i = name == "session_init") || name == "session_destroy")
        {
            eh = new SessionEventHandler();
            nHId = i ? EH_SESSION_INIT : EH_SESSION_DESTROY;
        }
        else
        {
            eh = CreateEventHandler();
            nHId = eh->StrToHandlerId(name);
        }

        PropertyObject propertyObject;
        propertyObject.transport = transportType;
        propertyObject.HandlerId = nHId;

        eh->init(params,propertyObject);
    } catch (SCAGException& e)
    {
        if (eh) delete eh;
        throw;
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
