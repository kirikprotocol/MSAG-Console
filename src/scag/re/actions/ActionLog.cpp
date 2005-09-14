#include "ActionLog.h"
#include "scag/re/CommandAdapter.h"

#include "scag/SAX2Print.hpp"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionLog::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'session:close': cannot have a child object");
}

bool ActionLog::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionLog::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (!params.Exists("level")) throw SCAGException("Action 'session:close': missing 'level' parameter");
    if (!params.Exists("category")) throw SCAGException("Action 'session:close': missing 'category' parameter");
    if (!params.Exists("message")) throw SCAGException("Action 'session:close': missing 'message' parameter");


    std::string sLevel = params["level"];

    if (sLevel == "error") level = lgError;
    else if (sLevel == "warn") level = lgWarning;
    else if (sLevel == "info") level = lgInfo;
    else if (sLevel == "debug") level = lgDebug;
    else throw SCAGException("Action 'log': invalid value for 'level' parameter");


    const char * name = 0;

    sCategory = params["category"];
    msg = params["message"];

    FieldType ft;
    AccessType at;

    ft = ActionContext::Separate(sCategory,name); 
    if (ft == ftUnknown) throw InvalidPropertyException("Action 'log': unrecognized variable prefix '%s' for 'category' parameter",sCategory.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'log': cannot read property '%s' - no access",sCategory.c_str());
    }

    ft = ActionContext::Separate(msg,name); 
    if (ft == ftUnknown) throw InvalidPropertyException("Action 'log': unrecognized variable prefix '%s' for 'message' parameter",msg.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'log': cannot read property '%s' - no access",sCategory.c_str());
    }

    smsc_log_debug(logger,"Action 'log':: init...");
}

bool ActionLog::run(ActionContext& context)
{
    Property * p1 = context.getProperty(sCategory);
    Property * p2 = context.getProperty(msg);

    if (!p1) 
    {
        smsc_log_warn(logger,"Action 'log': invalid property '%s' to log",sCategory.c_str());
        return true;
    }

    if (!p2) 
    {
        smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", msg.c_str());
        return true;
    }


    std::string logstr = p1->getStr() + ": " + p2->getStr();


    switch (level) 
    {
    case lgError: 
        smsc_log_error(logger,logstr);
        break;
    case lgWarning:
        smsc_log_warn(logger,logstr);
        break;

    case lgInfo:
        smsc_log_info(logger,logstr);
        break;
    case lgDebug:
        smsc_log_debug(logger,logstr);
        break;
    }


    return true;
}

ActionLog::~ActionLog()
{
}




}}}