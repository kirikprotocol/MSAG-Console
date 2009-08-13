#include "ActionSend2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/stat/base/Statistics2.h"

namespace scag2 {
namespace re {
namespace actions {

using namespace scag::stat;

static bool checkSDTPDateFormat(std::string& str, std::string& msg)
{
    char p, l;
    uint32_t i, y, m, d, h, min, s, t, nn;

    if(str.length() > 0)
    {
        i = sscanf(str.c_str(), "%2u%2u%2u%2u%2u%2u%1u%2u%c%c", &y, &m, &d, &h, &min, &s, &t, &nn, &p, &l);
        if(i < 9)
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
        else if(i > 9 && l != 'T' && l != 'S')
        {
            msg = "Wrong T parameter";
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

        val.assign(p2->getStr().c_str(), p2->getStr().size());
    } else
        val = str;
    return true;
}

IParserHandler * ActionSend::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    const char *p;
    bool bExist;
    std::string strTo, strFrom;
    FieldType ftTo, ftFrom;

    if(name == "send:sms")
    {
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, strTo, bExist);
        ftFrom = CheckParameter(params, propertyObject, "send", "from", false, true, strFrom, bExist);        
        try{
            if(ftTo == ftUnknown) Address a(strTo.c_str());
            if(ftFrom == ftUnknown && strFrom.length()) Address a1(strFrom.c_str());
        }
        catch(...)
        {
          throw SCAGException("Action 'send': invalid sms address %s or %s.", strTo.c_str(), strFrom.c_str());
        }
        toSms.Push(strTo);
        fromSms.Push(strFrom);        
    }else if (name == "send:email")
    {
        ftTo = CheckParameter(params, propertyObject, "send", "to", true, true, strTo, bExist);
        if(ftTo == ftUnknown && ((p = strchr(strTo.c_str(), '@')) == NULL || !*(p+1)))
               throw SCAGException("Action 'send': invalid recipient e-mail address %s. Should be name@domain", strTo.c_str());
        ftFrom = CheckParameter(params, propertyObject, "send", "from", false, true, strFrom, bExist);
        if(ftFrom == ftUnknown && strFrom.length() && ((p = strchr(strFrom.c_str(), '@')) == NULL || !*(p+1)))
               throw SCAGException("Action 'send': invalid sender e-mail address %s. Should be name@domain", strFrom.c_str());
        toEmail.Push(strTo);
        fromEmail.Push(strFrom);        
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

    propertyObject = _propertyObject;

    terminal = false;
    if(params.Exists("terminal") && !strcmp(params["terminal"].c_str(), "yes"))
        terminal = true;

    esmClass = 0;
    ftEsmClass = CheckParameter(params, propertyObject, "send", "esmClass", false, true, strEsmClass, bExist);
    if(ftEsmClass == ftUnknown && bExist)
        esmClass = atoi(strEsmClass.c_str());
        
    destPort = 0;
    ftDestPort = CheckParameter(params, propertyObject, "send", "destPort", false, true, strDestPort, bExist);
    if(ftDestPort == ftUnknown && bExist)
        destPort = atoi(strDestPort.c_str());
        
    srcPort = 0;
    ftSrcPort = CheckParameter(params, propertyObject, "send", "srcPort", false, true, strSrcPort, bExist);
    if(ftSrcPort == ftUnknown && bExist)
        srcPort = atoi(strSrcPort.c_str());
        
    ftPacketType = CheckParameter(params, propertyObject, "send", "packetType", false, true, strPacketType, bExist);
            
    // usr = false;
    // if(params.Exists("usr") && !strcmp(params["usr"].c_str(), "yes"))
    // usr = true;

    CheckParameter(params, propertyObject, "send", "message", true, true, strMsg, bExist);

    FieldType ft = CheckParameter(params, propertyObject, "send", "date", false, true, strDate, bExist);

    if(ft == ftUnknown && !checkSDTPDateFormat(strDate, msg))
        throw SCAGException("Action 'send' : invalid 'date' parameter: %s", msg.c_str());

    smsc_log_debug(logger,"Action 'send':: inited... level=%d", level);
}

bool ActionSend::run(ActionContext& context)
{
    const char *p;
    auto_ptr<SaccAlarmMessageEvent> ev(new SaccAlarmMessageEvent());
    std::string s2, s3;
    std::string msg;

    Statistics& sm = Statistics::Instance();

    if(!getStrProperty(context, strMsg, "message", ev->pMessageText))
        return true;

    for(int i = 0; i < toSms.Count(); i++)
    {
        s3.clear();
        if(!getStrProperty(context, toSms[i], "toSms", s2) || fromSms[i].length() && !getStrProperty(context, fromSms[i], "fromSms", s3))
            return true;

        try{
            Address a(s2.c_str());
            if(s3.length())
            {
                Address a1(s3.c_str());
                s3 = a1.toString().c_str();
            }
            ev->pAbonentsNumbers += a.toString().c_str();
            if(s3.length())
                ev->pAbonentsNumbers += "," + s3;
            ev->pAbonentsNumbers += ";";
        }
        catch(...)
        {
            smsc_log_warn(logger,"Action 'send': invalid message 'sms:to'='%s' or 'sms:from'='%s' property", s2.c_str(), s3.c_str());
        }
    }

    for(int i = 0; i < toEmail.Count(); i++)
    {
        s3.clear();
        if(!getStrProperty(context, toEmail[i], "toEmail", s2) || fromEmail[i].length() && !getStrProperty(context, fromEmail[i], "fromEmail", s3))
            return true;
            
        if(((p = strchr(s2.c_str(), '@')) == NULL || !*(p+1)))
               smsc_log_warn(logger, "Action 'send': invalid recipient e-mail address %s. Should be name@domain", s2.c_str());        
        else if(s3.length() && ((p = strchr(s3.c_str(), '@')) == NULL || !*(p+1)))
               smsc_log_warn(logger, "Action 'send': invalid sender e-mail address %s. Should be name@domain", s3.c_str());        
        else               
        {
            ev->pAddressEmail += s2;
            if(s3.length())
                ev->pAddressEmail += "," + s3;
            ev->pAddressEmail += ";";
        }
    }

    getStrProperty(context, strDate, "date", ev->pDeliveryTime);
    if(!checkSDTPDateFormat(ev->pDeliveryTime, msg))
    {
        smsc_log_error(logger, "Action 'send' invalid SDTP date format: %s", msg.c_str());
        return true;
    }
    
    Property * p2;
    if(ftDestPort != ftUnknown)
    {
        if(!(p2 = context.getProperty(strDestPort))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid 'destPort' property '%s'", strDestPort.c_str());
            return false;
        }
        destPort = uint16_t(p2->getInt());
    }
    
    if(ftSrcPort != ftUnknown)
    {
        if(!(p2 = context.getProperty(strSrcPort))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid 'srcPort' property '%s'", strSrcPort.c_str());
            return false;
        }
        srcPort = uint16_t(p2->getInt());
    }
    
    if(ftEsmClass != ftUnknown)
    {
        if(!(p2 = context.getProperty(strEsmClass))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid 'esmClass' property '%s'", strEsmClass.c_str());
            return false;
        }
        esmClass = uint8_t(p2->getInt());
    }
    
    if(ftPacketType != ftUnknown)
    {
        if(!(p2 = context.getProperty(strPacketType))) 
        {
            smsc_log_warn(logger,"Action 'send': invalid 'packetType' property '%s'", strPacketType.c_str());
            return false;
        }
        ev->pPacketType.assign(p2->getStr().c_str(), p2->getStr().size());
    }
    else
        ev->pPacketType = strPacketType;
    
    ev->sSrcPort = srcPort;
    ev->sDestPort = destPort;
    ev->cEsmClass = esmClass;

    ev->cCriticalityLevel = (uint8_t)level;
    
    // if (usr) ev->sUsr = context.getSession().getUSR();

    smsc_log_debug(logger, "msg: \"%s\", toEmail: \"%s\", toSms: \"%s\", date: \"%s\", esmClass: %d, destPort: %d, srcPort: %d, packetType: %s", ev->pMessageText.c_str(), ev->pAddressEmail.c_str(), ev->pAbonentsNumbers.c_str(), ev->pDeliveryTime.c_str(), (int)esmClass, destPort, srcPort, ev->pPacketType.c_str());

    sm.registerSaccEvent(ev.release());
    return true;
}

ActionSend::~ActionSend()
{
    //smsc_log_debug(logger, "'log' action released");
}

}}}
