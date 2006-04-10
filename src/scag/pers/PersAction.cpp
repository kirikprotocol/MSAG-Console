#include "PersAction.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/CommandAdapter.h"
#include "PersClient.h"
#include "Property.h"

namespace scag { namespace pers {

using namespace scag::stat;
using namespace scag::pers::client;

typedef scag::util::properties::Property REProperty;

const char* PersAction::getStrCmd()
{
    switch(cmd)
    {
        case PC_DEL:
            return "profile:del";
        case PC_SET:
            return "profile:set";
        case PC_GET:
            return "profile:get";
        case PC_INC:
            return "profile:change";
        default:
            return "unknown";
    }
}

ProfileType PersAction::getProfileTypeFromStr(std::string& str)
{
    if(!strcmp(str.c_str(), "ABONENT"))
        return PT_ABONENT;
    else if(!strcmp(str.c_str(), "OPERATOR"))
        return PT_OPERATOR;
    else if(!strcmp(str.c_str(), "PROVIDER"))
        return PT_PROVIDER;
    else if(!strcmp(str.c_str(), "SERVICE"))
        return PT_SERVICE;
    else
        return PT_UNKNOWN;
}

TimePolicy PersAction::getPolicyFromStr(std::string& str)
{
    if(!strcmp(str.c_str(), "INFINIT"))
        return INFINIT;
    else if(!strcmp(str.c_str(), "FIXED"))
        return FIXED;
    else if(!strcmp(str.c_str(), "ACCESS"))
        return ACCESS;
    else if(!strcmp(str.c_str(), "R_ACCESS"))
        return R_ACCESS;
    else if(!strcmp(str.c_str(), "W_ACCESS"))
        return W_ACCESS;
    else
        return UNKNOWN;
}

static std::wstring ConvertWStrTo_wstring(const std::string& str)
{
    TmpBuf<wchar_t, 1024> buf(1024);
    std::wstring wstr;
    const char* chrs  = str.c_str();

    for(int i = 0; i < str.length(); i+=2)
    {
        wchar_t r = (chrs[i] << 8) + chrs[i + 1];
        buf.Append(&r, 1);
    }

    wstr.assign(buf.get(), buf.GetPos());

    return wstr;
}

static std::string Convert_wstringToWstr(const std::wstring& wstr)
{
    std::string str;
    TmpBuf<char, 1024> buf(1024);
    const wchar_t* wchrs  = wstr.c_str();

    for(int i = 0; i < wstr.length(); i++)
    {
        char r = (*wchrs >> 8);
        buf.Append(&r, 1);
        r = *(wchrs++);
        buf.Append(&r, 1);
    }

    str.assign(buf.get(), buf.GetPos());
    return str;
}

void PersAction::init(const SectionParams& params, PropertyObject propertyObject)
{
    const char * name = 0;

    if(!logger) 
        logger = Logger::getInstance("scag.pers.action");

    if(!params.Exists("type") || (profile = getProfileTypeFromStr(ConvertWStrToStr(params["type"]))) == PT_UNKNOWN) 
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", getStrCmd());
    
    if(!params.Exists("var"))
        throw SCAGException("PersAction '%s' : missing 'var' parameter", getStrCmd());

    var = ConvertWStrToStr(params["var"]);

    if(cmd == PC_DEL)
        return;

    if(!params.Exists("value"))
        throw SCAGException("PersAction '%s' : missing 'value' parameter", getStrCmd());

    value_str = ConvertWStrToStr(params["value"]);
    value = ConvertWStrTo_wstring(params["value"]);

    ftValue = ActionContext::Separate(value_str, name); 
    if(cmd == PC_GET && (ftValue == ftUnknown || ftValue == ftConst))
        throw InvalidPropertyException("PersAction '%s': 'value' parameter should be an lvalue. Got %s", getStrCmd(), value_str.c_str());

    if(ftValue == ftField) 
    {
        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);
        if (!(at & atRead) || (cmd == PC_GET && !(at & atWrite))) 
            throw InvalidPropertyException("PersAction '%s': cannot read/modify property '%s' - no access", value_str.c_str());
    }

    if(cmd == PC_GET)
        return;

    if(!params.Exists("policy") || (policy = getPolicyFromStr(ConvertWStrToStr(params["policy"]))) == UNKNOWN)
        throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", getStrCmd());

    if(policy == INFINIT)
        return;

    if(policy == FIXED && params.Exists("finaldate"))
    {
        std::string dt = ConvertWStrToStr(params["finaldate"]);

        tm time;
        strptime(dt.c_str(), "%d.%m.%Y %T", &time);
        final_date = mktime(&time);
        return;
    }

    if(!params.Exists("lifetime"))
        throw SCAGException("PersAction '%s' : missing 'finaldate' or 'lifetime' parameter", getStrCmd());

    std::string lt = ConvertWStrToStr(params["lifetime"]);
    tm time;
    strptime(lt.c_str(), "%T", &time);
    life_time = mktime(&time);
}

uint32_t PersAction::getKey(const CommandProperty& cp, ProfileType pt)
{
    switch(pt)
    {
        case PT_SERVICE:
            return cp.serviceId;
        case PT_OPERATOR:
            return cp.operatorId;
        case PT_PROVIDER:
            return cp.providerId;
    }
    return 0;
}

static void setPersPropFromREProp(Property& prop, REProperty& rep)
{
    namespace reprop = scag::util::properties;

    switch(rep.getType())
    {
        case reprop::pt_int:
            prop.setIntValue(rep.getInt());
            break;
        case reprop::pt_bool:
            prop.setBoolValue(rep.getBool());
            break;
        case reprop::pt_date:
            prop.setDateValue(rep.getDate());
            break;
        case reprop::pt_str:
            prop.setStringValue(ConvertWStrTo_wstring(rep.getStr()).c_str());
            break;
    }
}

static void setREPropFromPersProp(REProperty& rep, Property& prop)
{
    switch(prop.getType())
    {
        case INT:
            rep.setInt(prop.getIntValue());
            break;
        case BOOL:
            rep.setBool(prop.getBoolValue());
            break;
        case DATE:
            rep.setDate(prop.getDateValue());
            break;
        case STRING:
            rep.setStr(Convert_wstringToWstr(prop.getStringValue()));
            break;
    }
}

bool PersAction::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'PersAction'...");

//    Statistics& statistics = Statistics::Instance();
//    SACC_BILLING_INFO_EVENT_t ev;

    try{
        Property prop;
        PersClient& pc = PersClient::Instance();

        CommandProperty& cp = context.getCommandProperty();
        switch(cmd)
        {
            case PC_DEL:
                if(profile == PT_ABONENT)
                    pc.DelProperty(profile, cp.abonentAddr.toString().c_str(), var.c_str());
                else
                    pc.DelProperty(profile, getKey(cp, profile), var.c_str());
                break;
            case PC_SET:
                if(ftValue != ftUnknown)
                {
                    REProperty *rp = context.getProperty(value_str);
                    setPersPropFromREProp(prop, *rp);
                    prop.setTimePolicy(policy, final_date, life_time);
                }
                else
                    prop.assign(var.c_str(), value.c_str(), policy, final_date, life_time);

                if(profile == PT_ABONENT)
                    pc.SetProperty(profile, cp.abonentAddr.toString().c_str(), prop);
                else
                    pc.SetProperty(profile, getKey(cp, profile), prop);
                break;
            case PC_GET:
            {
                if(profile == PT_ABONENT)
                    pc.GetProperty(profile, cp.abonentAddr.toString().c_str(), var.c_str(), prop);
                else
                    pc.GetProperty(profile, getKey(cp, profile), var.c_str(), prop);

                REProperty *rep = context.getProperty(value_str);
                setREPropFromPersProp(*rep, prop);
                break;
            }
            case PC_INC:
                if(profile == PT_ABONENT)
                    pc.IncProperty(profile, cp.abonentAddr.toString().c_str(), var.c_str(), 1);
                else
                    pc.IncProperty(profile, getKey(cp, profile), var.c_str(), 1);
                break;
        }
    }
    catch(PersClientException& e)
    {
        smsc_log_error(logger, "PersClientException catched, Reason: %s", e.what());
        return false;
    }

//    statistics.registerSaccEvent(ev);

    return true;
}

}}

