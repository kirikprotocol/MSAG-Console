#include "ActionSend.h"
#include "scag/re/CommandAdapter.h"
#include <scag/stat/Statistics.h>
#include <scag/stat/StatisticsManager.h>

namespace scag { namespace re { namespace actions {

using namespace scag::stat;

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
        if(ftTo == ftUnknown)
            Address a(strTo.c_str());
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
    return true;
}

void ActionSend::init(const SectionParams& params,PropertyObject _propertyObject)
{
    char *ptr;
    bool bExist;

    propertyObject = _propertyObject;

    ftMessage = CheckParameter(params, propertyObject, "send", "message", true, true, wstrMsg, bExist);
    strMsg = ConvertWStrToStr(wstrMsg);

    CheckParameter(params, propertyObject, "send", "date", true, true, wstrDate, bExist);
    strDate = ConvertWStrToStr(wstrDate);

    tm time;
    ptr = strptime(strDate.c_str(), "%Y%m%d%T", &time);
    if(!ptr || *ptr)
        throw SCAGException("Action 'send' : invalid 'date' parameter");

    smsc_log_debug(logger,"Action 'send':: inited... level=%d", level);
}

bool ActionSend::run(ActionContext& context)
{
    const char *name;
    SACC_ALARM_MESSAGE_t ev;
    Property * p1 = 0;
    Property * p2 = 0;
    std::string s2;
    FieldType ft;

    Statistics& sm = Statistics::Instance();

    if (ftMessage != ftUnknown)  
    {
        if (!(p2 = context.getProperty(strMsg))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid message property '%s'", strMsg.c_str());
            return true;
        }

        ev.pMessageText = ConvertWStrToStr(p2->getStr());
    } else
        ev.pMessageText = strMsg;

    for(int i = 0; i < toSms.Count(); i++)
    {
        if ((ft = ActionContext::Separate(toSms[i], name)) != ftUnknown)  
        {
            if (!(p2 = context.getProperty(toSms[i]))) 
            {
                smsc_log_warn(logger,"Action 'send': invalid toSms property '%s'", toSms[i].c_str());
                return true;
            }

            s2 = ConvertWStrToStr(p2->getStr());
        } 
        else
            s2 = toSms[i];

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
        if ((ft = ActionContext::Separate(toEmail[i], name)) != ftUnknown)
        {
            if (!(p2 = context.getProperty(toEmail[i]))) 
            {
                smsc_log_warn(logger,"Action 'send': invalid toEmail property '%s'", toEmail[i].c_str());
                return true;
            }

            s2 = ConvertWStrToStr(p2->getStr());
        } 
        else
            s2 = toSms[i];

        try{
            ev.pAddressEmail += s2 + ";";
        }
        catch(...)
        {
            smsc_log_warn(logger,"Action 'send': invalid message 'sms:to' property '%s'", s2.c_str());
        }
    }

    ev.pDeliveryTime = strDate;
    sm.registerSaccEvent(ev);
    return true;
}

ActionSend::~ActionSend()
{
    //smsc_log_debug(logger, "'log' action released");
}

}}}
