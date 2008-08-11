#include "ActionLog2.h"
#include "scag/re/CommandAdapter2.h"
#include "scag/util/lltostr.h"

namespace scag2 {
namespace re {
namespace actions {

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
    if(t == SMPP)
        return "smpp";
    else if(t == HTTP)
        return "http";
    else if(t == MMS)
        return "mms";
    return "unknown";
}

void ActionLog::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;

    if (!params.Exists("category")) throw SCAGException("Action 'log': missing 'category' parameter");

    const char * name = 0;
    strCategory = params["category"];
    std::string s = "rule." + ttToStr(propertyObject.transport);

    logger = Logger::getInstance(s.c_str());

    ftMessage = CheckParameter(params, propertyObject, "log", "message", true, true, strMsg, bExist);

    smsc_log_debug(logger,"Action 'log':: inited... level=%d, category %s, transport=%s", level, strCategory.c_str(), ttToStr(propertyObject.transport).c_str());
}

bool ActionLog::run(ActionContext& context)
{
    char buf[20];
    Property * p1 = 0;
    Property * p2 = 0;

    buf[19] = 0;
    std::string s2 = strCategory + "." + lltostr(context.getCommandProperty().serviceId, buf + 19) + ": ";

    if (ftMessage != ftUnknown)
    {
        if (!(p2 = context.getProperty(strMsg))) 
        {
            smsc_log_warn(logger,"Action 'log': invalid property '%s' to log", strMsg.c_str());
            return true;
        }

        s2 += p2->getStr();
    } 
    else
        s2 += strMsg;

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
