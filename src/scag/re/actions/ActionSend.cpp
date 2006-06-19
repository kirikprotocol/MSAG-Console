#include "ActionSend.h"
#include "scag/re/CommandAdapter.h"
#include <scag/stat/Statistics.h>
#include <scag/stat/StatisticsManager.h>

namespace scag { namespace re { namespace actions {

using namespace scag::stat;

static bool checkSDTPDateFormat(std::string& str, std::string& msg)
{
    char p;
    uint32_t i, y, m, d, h, min, s, t, nn;

    if(str.length() > 0)
    {
        i = sscanf(str.c_str(), "%2u%2u%2u%2u%2u%2u%1u%2u%c", &y, &m, &d, &h, &min, &s, &t, &nn, &p);
        if(i != 9)
        {
            msg = "Wrong field count";
            return false;
        }
        else if(m > 12)
        {
            msg = "Wrong month";
            return false;
        }
        else if((!d && p != 'R') || d > 31)
        {
            msg = "Wrong day";
            return false;
        }
        else if(h > 23 || min > 60 || s > 59)
        {
            msg = "Wrong time";
            return false;
        }
        else if(p != '-' && p != '+' && p != 'R')
        {
            msg = "Wrong p parameter";
            return false;
        }
        else if(nn > 48)
        {
            msg = "Wrong nn parameter";
            return false;
        }
    }
    return true;
}

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

        val = p2->getStr();
    } else
        val = str;
    return true;
}

IParserHandler * ActionSend::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    const char *p;
    bool bExist;
    std::string strTo;
    FieldType ftTo;

    if(name == "send:sms")
    {
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, strTo, bExist);
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
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, strTo, bExist);
        if(ftTo == ftUnknown && ((p = strchr(strTo.c_str(), '@')) == NULL || !*p))
               throw SCAGException("Action 'send': invalid e-mail address %s. Should be name@domain", strTo.c_str());
        toEmail.Push(strTo);
    } else
         throw SCAGException("Action 'send': unknown subsection %s", name.c_str());
    return NULL;
}

bool ActionSend::FinishXMLSubSection(const std::string& name)
{
    return ((name == "send:alarm")||(name == "send:info")||(name == "send:notify")||(name == "send:warn"));
}

void ActionSend::init(const SectionParams& params,PropertyObject _propertyObject)
{
    bool bExist;
    std::string msg;

    logger = Logger::getInstance("scag.re");

    propertyObject = _propertyObject;

    terminal = false;
    if(params.Exists("terminal"))
    {
        if(!strcmp(params["terminal"].c_str(), "yes"))
            terminal = true;
    }

    CheckParameter(params, propertyObject, "send", "message", true, true, strMsg, bExist);

    FieldType ft = CheckParameter(params, propertyObject, "send", "date", false, true, strDate, bExist);

    if(ft == ftUnknown && !checkSDTPDateFormat(strDate, msg))
        throw SCAGException("Action 'send' : invalid 'date' parameter: %s", msg.c_str());

    smsc_log_debug(logger,"Action 'send':: inited... level=%d", level);
}

bool ActionSend::run(ActionContext& context)
{
    SACC_ALARM_MESSAGE_t ev;
    std::string s2;
    std::string msg;

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

    getStrProperty(context, strDate, "date", ev.pDeliveryTime);
    if(!checkSDTPDateFormat(ev.pDeliveryTime, msg))
    {
        smsc_log_error(logger, "Action 'send' invalid SDTP date format: %s", msg.c_str());
        return true;
    }

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
