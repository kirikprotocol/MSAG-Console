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

/*
    if (!params.Exists("level")) throw SCAGException("Action 'log': missing 'level' parameter");
    if (!params.Exists("category")) throw SCAGException("Action 'log': missing 'category' parameter");
    if (!params.Exists("message")) throw SCAGException("Action 'log': missing 'message' parameter");
*/
    FieldType ft;
    std::string temp;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "log", "level", true, true, temp, bExist);
    std::string sLevel = ConvertWStrToStr(temp);

    if (sLevel == "error") level = lgError;
    else if (sLevel == "warn") level = lgWarning;
    else if (sLevel == "info") level = lgInfo;
    else if (sLevel == "debug") level = lgDebug;
    else throw SCAGException("Action 'log': invalid value for 'level' parameter");

    ftCategory = CheckParameter(params, propertyObject, "log", "category", true, true, wstrCategory, bExist);
    strCategory = ConvertWStrToStr(wstrCategory);

    ftMessage = CheckParameter(params, propertyObject, "log", "message", true, true, wstrMsg, bExist);
    strMsg = ConvertWStrToStr(wstrMsg);


    smsc_log_debug(logger,"Action 'log':: init...");
}

bool ActionLog::run(ActionContext& context)
{
    Property * p1 = 0;
    Property * p2 = 0;

    std::string s1,s2;
    std::string wstr;

    if (ftCategory != ftUnknown) 
    {
        p1 = context.getProperty(strCategory);
        if (!p1) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", strCategory.c_str());
            return true;
        }
        wstr = p1->getStr();
        s1 = FormatWStr(wstr);
    } else s1 = FormatWStr(wstrCategory);

    if (ftMessage!=ftUnknown)  
    {
        p2 = context.getProperty(strMsg);

        if (!p2) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", FormatWStr(wstrMsg).c_str());
            return true;
        }

        wstr = p2->getStr();
        s2 = FormatWStr(wstr);

    } else s2 = FormatWStr(wstrMsg);

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