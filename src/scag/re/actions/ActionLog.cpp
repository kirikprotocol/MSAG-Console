#include "ActionLog.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

IParserHandler * ActionLog::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'log': cannot have a child object");
}

bool ActionLog::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionLog::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");


    if (!params.Exists("level")) throw SCAGException("Action 'log': missing 'level' parameter");
    if (!params.Exists("category")) throw SCAGException("Action 'log': missing 'category' parameter");
    if (!params.Exists("message")) throw SCAGException("Action 'log': missing 'message' parameter");


    std::string sLevel = ConvertWStrToStr(params["level"]);

    if (sLevel == "error") level = lgError;
    else if (sLevel == "warn") level = lgWarning;
    else if (sLevel == "info") level = lgInfo;
    else if (sLevel == "debug") level = lgDebug;
    else throw SCAGException("Action 'log': invalid value for 'level' parameter");


    const char * name = 0;
    w_Category = params["category"].c_str();
    s_Category = ConvertWStrToStr(w_Category);

    w_msg = params["message"];
    s_msg = ConvertWStrToStr(w_msg);

    AccessType at;

    ftCategory = ActionContext::Separate(s_Category,name); 
    //if (ft == ftUnknown) throw InvalidPropertyException("Action 'log': unrecognized variable prefix '%s' for 'category' parameter",sCategory.c_str());

    if (ftCategory == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'log': cannot read property '%s' - no access",s_Category.c_str());
    }

    ftMessage = ActionContext::Separate(s_msg,name); 
    //if (ft == ftUnknown) throw InvalidPropertyException("Action 'log': unrecognized variable prefix '%s' for 'message' parameter",msg.c_str());

    if (ftMessage == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'log': cannot read property '%s' - no access",s_msg.c_str());
    }

    smsc_log_debug(logger,"Action 'log':: init...");
}

bool ActionLog::run(ActionContext& context)
{
    Property * p1 = 0;
    Property * p2 = 0;

    std::string s1,s2;
    std::wstring wstr;

    if (ftCategory != ftUnknown) 
    {
        p1 = context.getProperty(s_Category);
        if (!p1) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", s_Category.c_str());
            return true;
        }
        wstr = p1->getStr();
        s1 = FormatWStr(wstr);
    } else s1 = FormatWStr(w_Category);

    if (ftMessage!=ftUnknown)  
    {
        p2 = context.getProperty(s_msg);

        if (!p2) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", s_msg.c_str());
            return true;
        }

        wstr = p2->getStr();
        s2 = FormatWStr(wstr);

    } else s2 = FormatWStr(w_msg);

    std::string logstr = s1 + ": " + s2;

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
    //smsc_log_debug(logger, "'log' action released");
}




}}}