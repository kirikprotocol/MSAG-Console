#include "PersAction.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/CommandAdapter.h"
#include "scag/pers/PersClient.h"
#include "scag/pers/Property.h"

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
            return "profile:inc";
        case PC_INC_MOD:
            return "profile:inc-mod";
        default:
            return "unknown";
    }
}

ProfileType PersAction::getProfileTypeFromStr(const std::string& str)
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

TimePolicy PersAction::getPolicyFromStr(const std::string& str)
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

void PersAction::init(const SectionParams& params, PropertyObject propertyObject)
{
    const char * name = 0;

    if(!logger) 
        logger = Logger::getInstance("scag.pers.action");

    if(!params.Exists("type") || (profile = getProfileTypeFromStr(params["type"])) == PT_UNKNOWN) 
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", getStrCmd());
    
    if(!params.Exists("var"))
        throw SCAGException("PersAction '%s' : missing 'var' parameter", getStrCmd());

    var = params["var"];

    if(cmd == PC_DEL)
    {
        smsc_log_debug(logger, "act params: cmd = %s, profile=%d, var=%s", getStrCmd(), profile, var.c_str());
        return;
    }

    if(cmd == PC_INC_MOD || cmd == PC_INC)
        value_str = params.Exists("inc") ? params["inc"] : "1";
    else
    {
        if(!params.Exists("value"))
            throw SCAGException("PersAction 'value' : missing '%s' parameter", getStrCmd());
        value_str = params["value"];
    }

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
    {
        smsc_log_debug(logger, "act params: cmd = %s, profile=%d, var=%s", getStrCmd(), profile, var.c_str());
        return;
    }

    if(cmd == PC_INC_MOD)
    {
        mod = 0;
        mod_str = params.Exists("mod") ? params["mod"] : "0";

        ftModValue = ActionContext::Separate(mod_str, name); 
        if(ftModValue == ftField) 
        {
            AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);
            if(!(at & atRead)) 
                throw InvalidPropertyException("PersAction '%s': cannot read property '%s' - no access", mod_str.c_str());
        }

        if(ftModValue == ftUnknown && strcmp(mod_str.c_str(), "0") && !(mod = atoi(mod_str.c_str())))
            throw SCAGException("PersAction '%s' : 'mod' parameter not a number. mod=%s", getStrCmd(), mod_str.c_str());

        if(!params.Exists("result"))
            throw SCAGException("PersAction '%s' : missing 'result' parameter", getStrCmd());

        result_str = params["result"];

        FieldType ftResultValue = ActionContext::Separate(result_str, name);
        if(ftResultValue == ftUnknown || ftValue == ftConst)
            throw InvalidPropertyException("PersAction '%s': 'result' parameter should be an lvalue. Got %s", getStrCmd(), value_str.c_str());

        if(ftResultValue == ftField)
        {
            AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);
            if(!(at & atWrite))
                throw InvalidPropertyException("PersAction '%s': cannot modify property '%s' - no access", value_str.c_str());
        }
    }

    if(!params.Exists("policy") || (policy = getPolicyFromStr(params["policy"])) == UNKNOWN)
        throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", getStrCmd());

    if(policy == INFINIT)
    {
        smsc_log_debug(logger, "act params: cmd = %s, profile=%d, var=%s, policy=%d, mod=%d", getStrCmd(), profile, var.c_str(), policy, mod);
        return;
    }

    if(policy == FIXED && params.Exists("finaldate"))
    {
        std::string dt = params["finaldate"];

        struct tm time;
        char *ptr;

        ptr = strptime(dt.c_str(), "%d.%m.%Y %T", &time);
        if(!ptr || *ptr)
            throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", getStrCmd());

        final_date = mktime(&time);
        smsc_log_debug(logger, "act params: cmd = %s, profile=%d, var=%s, policy=%d, final_date=%d(%s)", getStrCmd(), profile, var.c_str(), policy, final_date, dt.c_str());
        return;
    }

    if(!params.Exists("lifetime"))
        throw SCAGException("PersAction '%s' : missing 'finaldate' or 'lifetime' parameter", getStrCmd());

    std::string lt = params["lifetime"];

    struct tm time;
    char *ptr;

    ptr = strptime(lt.c_str(), "%H:%M:%S", &time);
    if(!ptr || *ptr)
        throw SCAGException("PersAction '%s' : invalid 'lifetime' parameter", getStrCmd());

    life_time = time.tm_hour * 3600 + time.tm_min * 60 + time.tm_sec;

    smsc_log_debug(logger, "act params: cmd = %s, profile=%d, var=%s, policy=%d, life_time=%d(%s), mod=%d", getStrCmd(), profile, var.c_str(), policy, life_time, lt.c_str(), mod);
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
            prop.setStringValue(rep.getStr().c_str());
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
            rep.setStr(prop.getStringValue());
            break;
    }
}

bool PersAction::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s. var=%s'...", getStrCmd(), var.c_str());

    try{
        Property prop;
        PersClient& pc = PersClient::Instance();

        CommandProperty& cp = context.getCommandProperty();

        switch(cmd)
        {
            case PC_DEL:
                if(profile == PT_ABONENT)
                    pc.DelProperty(profile, cp.abonentAddr.value, var.c_str());
                else
                    pc.DelProperty(profile, getKey(cp, profile), var.c_str());
                break;
            case PC_INC:
            case PC_INC_MOD:
            case PC_SET:
                if(ftValue != ftUnknown)
                {
                    REProperty *rp = context.getProperty(value_str);
                    if(rp)
                    {
                        setPersPropFromREProp(prop, *rp);
                        prop.setName(var);
                        prop.setTimePolicy(policy, final_date, life_time);
                    } else
                        return false;
                }
                else
                    prop.assign(var.c_str(), value_str.c_str(), policy, final_date, life_time);

                if(cmd == PC_SET)
                {
                    if(profile == PT_ABONENT)
                        pc.SetProperty(profile, cp.abonentAddr.value, prop);
                    else
                        pc.SetProperty(profile, getKey(cp, profile), prop);
                }
                else if(cmd == PC_INC)
                {
                    if(profile == PT_ABONENT)
                        pc.IncProperty(profile, cp.abonentAddr.value, prop);
                    else
                        pc.IncProperty(profile, getKey(cp, profile), prop);
                }
                else if(cmd == PC_INC_MOD)
                {
                    if(ftModValue != ftUnknown)
                    {
                        REProperty *rp = context.getProperty(mod_str);
                        if(!rp)
                            return false;
                        mod = rp->getInt();
                    }

                    int res;
                    if(profile == PT_ABONENT)
                        res = pc.IncModProperty(profile, cp.abonentAddr.value, prop, mod);
                    else
                        res = pc.IncModProperty(profile, getKey(cp, profile), prop, mod);

                    REProperty *rep = context.getProperty(result_str);
                    rep->setInt(res);
                }

                break;
            case PC_GET:
            {
                try{
                    if(profile == PT_ABONENT)
                        pc.GetProperty(profile, cp.abonentAddr.value, var.c_str(), prop);
                    else
                        pc.GetProperty(profile, getKey(cp, profile), var.c_str(), prop);
                    REProperty *rep = context.getProperty(value_str);
                    setREPropFromPersProp(*rep, prop);
                }
                catch(PersClientException& e)
                {
                    if(e.getType() == PROPERTY_NOT_FOUND)
                    {
                        REProperty *rep = context.getProperty(value_str);
                        rep->setStr("");
                        smsc_log_warn(logger, "PersClientException: GetProperty not found: %s", var.c_str());
                    }
                    else
                        throw e;
                }
                break;
            }
        }
    }
    catch(PersClientException& e)
    {
        if(cmd == PC_DEL && e.getType() == PROPERTY_NOT_FOUND)
        {
            smsc_log_warn(logger, "PersClientException: Property not found on deletion: %s", var.c_str());
            return true;
        }
        smsc_log_error(logger, "PersClientException: var=%s, reason: %s", var.c_str(), e.what());
        return false;
    }

    return true;
}

IParserHandler * PersAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'profile:': cannot have a child object");
}

bool PersAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}

