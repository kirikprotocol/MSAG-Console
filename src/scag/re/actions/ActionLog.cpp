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

std::string ActionLog::ttToStr(TransportType t)
{
    if(t == 1)
        return "smpp";
    else if(t == 2)
        return "http";
    else if(t == 3)
        return "mms";
    return "unknown";
}

void ActionLog::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;

    if (!params.Exists("category")) throw SCAGException("Action 'log': missing 'category' parameter");

    const char * name = 0;
    strCategory = ConvertWStrToStr(params["category"]);
    std::string s = "rule." + ttToStr(propertyObject.transport) + "." + strCategory;

    logger = Logger::getInstance(s.c_str());

    ftMessage = CheckParameter(params, propertyObject, "log", "message", true, true, wstrMsg, bExist);
    strMsg = ConvertWStrToStr(wstrMsg);

    smsc_log_debug(logger,"Action 'log':: inited... level=%d, category %s, transport=%s", level, strCategory.c_str(), ttToStr(propertyObject.transport).c_str());
}

bool ActionLog::run(ActionContext& context)
{
    Property * p1 = 0;
    Property * p2 = 0;

    std::string s2;

    if (ftMessage != ftUnknown)  
    {
        if (!(p2 = context.getProperty(strMsg))) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", strMsg.c_str());
            return true;
        }

        s2 = ConvertWStrToStr(p2->getStr());
    } 
    else
        s2 = strMsg;

    switch (level) 
    {
        case lgError: smsc_log_error(logger,s2); break;
        case lgWarning: smsc_log_warn(logger,s2); break;
        case lgInfo: smsc_log_info(logger,s2); break;
        case lgDebug: smsc_log_debug(logger,s2); break;
    }

    return true;
}

ActionLog::~ActionLog()
{
    //smsc_log_debug(logger, "'log' action released");
}

}}}