#include "ActionTLV.h"
#include "scag/re/CommandAdapter.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace actions {

Hash<int> ActionTLV::namesHash = ActionTLV::InitNames();

Hash<int> ActionTLV::InitNames()
{
    Hash<int> hs;    
    hs["name1"] = 11;
    return hs;
}

void ActionTLV::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    bool bExist;

    tag = 0;
    ftTag = CheckParameter(params, propertyObject, "tlv", "tag", false, true, strTag, byTag);
    if(!byTag)
    {
        ftTag = CheckParameter(params, propertyObject, "tlv", "name", false, true, strTag, bExist);
        if(ftTag == ftUnknown)
        {
            int *p = namesHash.GetPtr(strTag.c_str());
            if(!p)
                throw SCAGException("Action 'tlv': Invalid NAME value");            
            tag = *p;
        }
    }
    else
    {
        if(ftTag == ftUnknown)
        {
            tag = atoi(strTag.c_str());
            if(!tag)
                throw SCAGException("Action 'tlv': Invalid TAG value");
        }
    }
    
    if(type == TLV_SET || type == TLV_GET)
    {
        ftVar = CheckParameter(params, propertyObject, "tlv", "var", true, type == TLV_SET, strVar, bExist);
        if(type == TLV_GET && (ftVar == ftUnknown || ftVar == ftConst))
            throw SCAGException("Action 'tlv': Get var can't be a const value");        
        if(ftVar == ftUnknown && (tag >> 8) == SMS_INT_TAG)
        {
            val = atoi(strVar.c_str());
            if(!val && (strVar[0] != '0' || strVar.length() != 1))
                throw SCAGException("Action 'tlv': Invalid value for integer TAG %d, var=%s", tag, strVar.c_str());
        }
    }
    else if(type == TLV_EXIST)
    {
        ftResult = CheckParameter(params, propertyObject, "tlv", "result", true, false, strVar, bExist);
        if(ftResult == ftUnknown || ftResult == ftConst)
            throw SCAGException("Action 'tlv': Result can't be a const value");
    }

    smsc_log_debug(logger,"Action 'tlv':: init");
}

bool ActionTLV::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'tlv'");

    SMS& sms = CommandBrige::getSMS((SmppCommand&)context.getSCAGCommand());

    smsc_log_debug(logger, "Action SMS %x", &sms);        
    
    if(!tag)
    {
        Property* property = context.getProperty(strTag);
        if(byTag)
        {
            tag = atoi(strTag.c_str());
            if(!tag)
                throw SCAGException("Action 'tlv': Invalid TAG value");
        }
        else
        {
            int *p = namesHash.GetPtr(strTag.c_str());
            if(!p)
                throw SCAGException("Action 'tlv': Invalid NAME value");
            tag = *p;
        }
    }
    
    int tt = tag >> 8;
    tag &= 0xFF;

    Property* prop = NULL;
    if(type != TLV_DEL && ftVar != ftUnknown)
    {
        prop = context.getProperty(strVar);
        if (!prop) 
        {
            smsc_log_warn(logger,"Action 'tlv':: invalid result property '%s'",strVar.c_str());
            return true;
        }
    }
        
    if(type == TLV_EXIST)
    {
        prop->setBool(sms.hasProperty(tag));
        smsc_log_debug(logger, "Action 'tlv': Tag: %d is %s", tag, prop->getBool() ? "set" : "not set");
    }
    else if(type == TLV_DEL)
    {
        sms.dropProperty(tag);
        smsc_log_debug(logger, "Action 'tlv': Tag: %d deleted", tag);
    }
    else if(type == TLV_GET)
    {
        if(!sms.hasProperty(tag))
        {
            smsc_log_warn(logger, "Action 'tlv': Get of not set tag %d.", tag);
            return true;
        }
        if(tt == SMS_INT_TAG)
        {
            prop->setInt(sms.getIntProperty(tag));
            smsc_log_debug(logger, "sss: %d %d", sms.getIntProperty(tag), prop->getInt());            
            smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%d", tag, prop->getInt());
        }
        else if(tt == SMS_STR_TAG)
        {
            prop->setStr(sms.getStrProperty(tag | (SMS_STR_TAG << 8)));
            smsc_log_debug(logger, "Action 'tlv': Tag: %d. GetValue=%s", tag, prop->getStr().c_str());
        }
    
    }
    else if(type == TLV_SET)
    {
        if(tt == SMS_INT_TAG)
        {
            sms.setIntProperty(tag, prop ? prop->getInt() : val);
            smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%d", tag, prop ? prop->getInt() : val);
        }
        else if(tt == SMS_STR_TAG)
        {
            sms.setStrProperty(tag, prop ? prop->getStr().c_str() : strVar.c_str());
            smsc_log_debug(logger, "Action 'tlv': Tag: %d. SetValue=%s", tag, prop ? prop->getStr().c_str() : strVar.c_str());
        }
    }

    return true;
}

IParserHandler * ActionTLV::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'tlv' cannot include child objects");
}

bool ActionTLV::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
