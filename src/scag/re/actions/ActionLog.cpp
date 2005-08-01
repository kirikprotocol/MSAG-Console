#include "ActionLog.h"
#include "scag/re/SAX2Print.hpp"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionLog::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw Exception("Action 'session:close': cannot have a child object");
}

bool ActionLog::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionLog::init(const SectionParams& params)
{
    if (!params.Exists("level")) throw Exception("Action 'session:close': missing 'level' parameter");
    if (!params.Exists("category")) throw Exception("Action 'session:close': missing 'category' parameter");
    if (!params.Exists("message")) throw Exception("Action 'session:close': missing 'message' parameter");


    std::string sLevel = params["level"];

    if (sLevel == "error") level = lgError;
    else if (sLevel == "warn") level = lgWarning;
    else if (sLevel == "info") level = lgInfo;
    else if (sLevel == "debug") level = lgDebug;
    else throw Exception("Action 'session:close': invalid value for 'level' parameter");


    sCategory = params["category"];
    msg = params["message"];

}

bool ActionLog::run(ActionContext& context)
{

    switch (level) 
    {
    case lgError: 
        smsc_log_error(logger,msg);
        break;
    case lgWarning:
        smsc_log_warn(logger,msg);
        break;

    case lgInfo:
        smsc_log_info(logger,msg);
        break;
    case lgDebug:
        smsc_log_debug(logger,msg);
        break;
    }


    return true;
}

ActionLog::~ActionLog()
{
}




}}}