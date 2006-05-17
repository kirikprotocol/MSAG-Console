#include "ActionSend.h"
#include "scag/re/CommandAdapter.h"
#include <scag/stat/Statistics.h>
#include <scag/stat/StatisticsManager.h>

namespace scag { namespace re { namespace actions {

using namespace scag::stat;

bool ActionSend::getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val)
{
    const char *name;
    Property * p2 = 0;
    FieldType ft = ActionContext::Separate(str, name);
    if (ft != ftUnknown)  
    {
        if (!(p2 = context.getProperty(str))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid '%s' property '%s'", field_name, str.c_str());
            return false;
        }

        val = ConvertWStrToStr(p2->getStr());
    } else
        val = str;
    return true;
}

IParserHandler * ActionSend::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    const char *p;
    bool bExist;
    std::string strTo, wstrTo;
    FieldType ftTo;

    if(name == "send:sms")
    {
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, wstrTo, bExist);
        strTo = ConvertWStrToStr(wstrTo);
        try{
        if(ftTo == ftUnknown)
            Address a(strTo.c_str());
        }
        catch(...)
        {
          throw SCAGException("Action 'send': invalid sms address %s.", strTo.c_str());
        }
        toSms.Push(strTo);
    }else if (name == "send:email")
    {
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, wstrTo, bExist);
        strTo = ConvertWStrToStr(wstrTo);
        if(ftTo == ftUnknown && ((p = strchr(strTo.c_str(), '@')) == NULL || !*p))
               throw SCAGException("Action 'send': invalid e-mail address %s. Should be name@domain", strTo.c_str());
        toEmail.Push(strTo);
    } else
         throw SCAGException("Action 'send': unknown subsection %s", name.c_str());
    return NULL;
}

bool ActionSend::FinishXMLSubSection(const std::string& name)
{
    return (name == "send:alarm");
}

void ActionSend::init(const SectionParams& params,PropertyObject _propertyObject)
{
    uint32_t i, y, m, d, h, min, s, t, nn;
    char p;
    char *ptr;
    bool bExist;
    std::string str;

    logger = Logger::getInstance("scag.re");

    propertyObject = _propertyObject;

    terminal = false;
    if(params.Exists("terminal"))
    {
        str = ConvertWStrToStr(params["terminal"]);
        if(!strcmp(str.c_str(), "yes"))
            terminal = true;
    }

    CheckParameter(params, propertyObject, "send", "message", true, true, wstrMsg, bExist);
    strMsg = ConvertWStrToStr(wstrMsg);

    CheckParameter(params, propertyObject, "send", "date", true, true, wstrDate, bExist);
    strDate = ConvertWStrToStr(wstrDate);

    i = sscanf(strDate.c_str(), "%2u%2u%2u%2u%2u%2u%1u%2u%c", &y, &m, &d, &h, &min, &s, &t, &nn, &p);
    if(i < 9 || m > 12 || !d || d > 31 || h > 23 || min > 60 || s > 59 || (p != '-' && p != '+'))
        throw SCAGException("Action 'send' : invalid 'date' parameter");

    smsc_log_debug(logger,"Action 'send':: inited... level=%d", level);
}

bool ActionSend::run(ActionContext& context)
{
    SACC_ALARM_MESSAGE_t ev;
    std::string s2;

    Statistics& sm = Statistics::Instance();

    if(!getStrProperty(context, strMsg, "message", ev.pMessageText))
        return true;

    for(int i = 0; i < toSms.Count(); i++)
    {
        if(!getStrProperty(context, toSms[i], "toSms", s2))
            return true;

        try{
            Address a(s2.c_str());
            ev.pAbonentsNumbers += a.toString() + ";";
        }
        catch(...)
        {
            smsc_log_warn(logger,"Action 'send': invalid message 'sms:to' property '%s'", s2.c_str());
        }
    }

    for(int i = 0; i < toEmail.Count(); i++)
    {
        if(!getStrProperty(context, toEmail[i], "toEmail", s2))
            return true;

        ev.pAddressEmail += s2 + ";";
    }

    if(!getStrProperty(context, strDate, "date", ev.pDeliveryTime))
        return true;

    ev.cCriticalityLevel = (uint8_t)level;

    smsc_log_debug(logger, "msg: %s, toEmail: %s, toSms: %s, date: %s", ev.pMessageText.c_str(), ev.pAddressEmail.c_str(), ev.pAbonentsNumbers.c_str(), ev.pDeliveryTime.c_str());

    sm.registerSaccEvent(ev);
    return true;
}

ActionSend::~ActionSend()
{
    //smsc_log_debug(logger, "'log' action released");
}

}}}
