#include "PersAction.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/CommandAdapter.h"
#include "scag/pers/Property.h"

#include "sms/sms.h"

namespace scag { namespace pers {

typedef scag::util::properties::Property REProperty;

using namespace scag::stat;

char buf; // crashs without this :)

const char* OPTIONAL_KEY = "key";

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

time_t PersAction::parseFinalDate(const std::string& s)
{
	struct tm time;
    char *ptr;

    ptr = strptime(s.c_str(), "%d.%m.%Y %T", &time);
    if(!ptr || *ptr)
		return 0;

    return mktime(&time);
}

uint32_t PersAction::parseLifeTime(const std::string& s)
{
	uint32_t hour = 0, min = 0, sec = 0;
    if(sscanf(s.c_str(), "%u:%u:%u", &hour, &min, &sec) < 3)
		return 0;

    return hour * 3600 + min * 60 + sec;
}

void PersAction::getOptionalKey(const std::string& key_str) {
  if (profile == PT_ABONENT) {
    try {
      optional_skey = getAbntAddress(key_str);
    } catch(const std::runtime_error& e) {
        throw SCAGException("PersAction '%s' : '%s' parameter has error abonent profile key: %s",
                            getStrCmd(), OPTIONAL_KEY, e.what());
    }
  } else if (strcmp(key_str.c_str(), "0") && !(optional_ikey = atoi(key_str.c_str()))) {
      throw SCAGException("PersAction '%s' : '%s' parameter not a number in not abonent profile type. key=%s",
                           getStrCmd(), OPTIONAL_KEY, key_str.c_str());
  }
}

void PersAction::init(const SectionParams& params, PropertyObject propertyObject)
{
	bool bExist;
    const char * name = 0;

    switch(cmd)
    {
        case PC_DEL: lcm_cmd = PERS_DEL; break;
        case PC_INC: lcm_cmd = PERS_INC; break;
        case PC_INC_MOD: lcm_cmd = PERS_INC_MOD; break;
        case PC_SET: lcm_cmd = PERS_SET; break;
        case PC_GET: lcm_cmd = PERS_GET; break;
    }
        
    if(!params.Exists("type") || (profile = getProfileTypeFromStr(params["type"])) == PT_UNKNOWN) 
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", getStrCmd());
    
    varType = CheckParameter(params, propertyObject, "PersAction", "var", true, true, var, bExist);
/////////////////////////////////////////////
    bool statusExist = false;
    CheckParameter(params, propertyObject, "PersAction", "status", false, false,
                    status_str, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, "PersAction", "msg", false, false,
                    msg_str, msgExist);
//////////////////////////////////////////////
    ftOptionalKey = CheckParameter(params,propertyObject, "PersAction", OPTIONAL_KEY, false, true,
                                       optional_key_str, has_optional_key);
    if (has_optional_key && ftOptionalKey == ftUnknown) {
      smsc_log_debug(logger, "PersAction: optional_key_str=%s, optional key exists",
                      optional_key_str.c_str(), has_optional_key);
      getOptionalKey(optional_key_str);
    }
// /////////////////
    if(cmd == PC_DEL)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, profile=%d, var=%s", getStrCmd(), profile, var.c_str());
        return;
    }

    if(cmd == PC_INC_MOD || cmd == PC_INC) {
      value_str = params.Exists("inc") ? params["inc"] : "1";
      bool resultExist = false;
      CheckParameter(params, propertyObject, "PersAction", "result", false, false, result_str, resultExist);
    }
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
        smsc_log_debug(logger, "PersAction: params: cmd = %s, profile=%d, var=%s", getStrCmd(), profile, var.c_str());
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
    }

    if(!params.Exists("policy") || (policy = getPolicyFromStr(params["policy"])) == UNKNOWN)
        throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", getStrCmd());

    if(policy == INFINIT)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, profile=%d, var=%s, policy=%d, mod=%d", getStrCmd(), profile, var.c_str(), policy, mod);
        return;
    }

    if(policy == FIXED && params.Exists("finaldate"))
    {
	
	    finalDateFieldType = CheckParameter(params, propertyObject, "PersAction", "finaldate", true, true, sFinalDate, bExist);
	    if(finalDateFieldType == ftUnknown)
	    {
			final_date = parseFinalDate(sFinalDate);
	        if(!final_date)
    	        throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", getStrCmd());
	    }
	
        smsc_log_debug(logger, "PersAction: params: cmd = %s, profile=%d, var=%s, policy=%d, final_date=%d", getStrCmd(), profile, var.c_str(), policy, final_date);
        return;
    }

    lifeTimeFieldType = CheckParameter(params, propertyObject, "PersAction", "lifetime", true, true, sLifeTime, bExist);
    if(lifeTimeFieldType == ftUnknown)
    {
		life_time = parseLifeTime(sLifeTime);
	    if(!life_time)
    	    throw SCAGException("PersAction '%s' : invalid 'lifetime' parameter", getStrCmd());
    }

    smsc_log_debug(logger, "PersAction: params: cmd = %s, profile=%d, var=%s, policy=%d, life_time=%d, mod=%d, optional key='%s'",
                    getStrCmd(), profile, var.c_str(), policy, life_time, mod, optional_key_str.c_str());
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
          prop.setValue(rep.getStr().c_str());
          //prop.setStringValue(rep.getStr().c_str());
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

static uint32_t cmdToLongCallCmd(uint32_t c)
{
    switch(c)
    {
        case PC_DEL: return PERS_DEL;
        case PC_SET: return PERS_SET;
        case PC_GET: return PERS_GET;
        case PC_INC: return PERS_INC;
        case PC_INC_MOD: return PERS_INC_MOD;
    }
    return 0;
}

std::string PersAction::getAbntAddress(const std::string& _address) {
  Address address(_address.c_str());
  if (_address[0] != '.') {
    address.setNumberingPlan(1);
    address.setTypeOfNumber(1);
  }
  return address.toString();
}

bool PersAction::batchPrepare(ActionContext& context, SerialBuffer& sb)
{
	std::string skey;
	Property prop;
	uint32_t md;
	time_t fd = final_date, lt = life_time;
    smsc_log_debug(logger,"Run BatchPrepare 'PersAction cmd=%s. var=%s'...", getStrCmd(), var.c_str());

    CommandProperty& cp = context.getCommandProperty();

    REProperty *p;
	if(varType != ftUnknown && !(p = context.getProperty(var)))
	{
		smsc_log_error(logger, "Invalid var property  %s", var.c_str());
		return false;
	}
	const std::string& svar = varType == ftUnknown ? var : p->getStr();

    PersKey pk;
    if (has_optional_key) {
      if (ftOptionalKey == PT_UNKNOWN) {
        if (profile == PT_ABONENT) {
          pk.skey = optional_skey.c_str();
          skey = optional_skey;
        } else {
          pk.ikey = optional_ikey;
        }
      } else {
        REProperty *rp = context.getProperty(optional_key_str);
        if(!rp) {
          smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                          OPTIONAL_KEY, optional_key_str.c_str());
          return false;
        }
        if (profile == PT_ABONENT) {
          try {
            skey = getAbntAddress(rp->getStr());
            pk.skey = skey.c_str();
          } catch(const std::runtime_error& e) {
            smsc_log_error(logger, "'%s' parameter has error abonent profile key: %s", OPTIONAL_KEY, e.what());
            return false;
          }
        } else {
          pk.ikey = rp->getInt();
        }
      }
    } else {
      if(profile == PT_ABONENT)
      {
          skey = cp.abonentAddr.toString();
          pk.skey = skey.c_str();
      }
      else
          pk.ikey = getKey(cp, profile);
    }
		
	if(lifeTimeFieldType != ftUnknown)
	{
        REProperty *rp = context.getProperty(sLifeTime);
        if(!rp || !(lt = parseLifeTime(rp->getStr())))
		{
			smsc_log_error(logger, "Invalid lifeTime parameter %s(%s)", sLifeTime.c_str(), rp ? rp->getStr().c_str() : "");
			return false;
		}
	}
	if(finalDateFieldType != ftUnknown)
	{
        REProperty *rp = context.getProperty(sFinalDate);
        if(!rp || !(fd = parseFinalDate(rp->getStr())))
		{
			smsc_log_error(logger, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
			return false;
		}
	}
		
    if(cmd == PC_INC || cmd == PC_INC_MOD || cmd == PC_SET)
    {
        if(ftValue != ftUnknown)
        {
            REProperty *rp = context.getProperty(value_str);
            if(rp)
            {
                setPersPropFromREProp(prop, *rp);
                prop.setName(svar);
                prop.setTimePolicy(policy, fd, lt);
            }
            else 
            {
                smsc_log_error(logger, "parameter '%s' not found in action context", value_str.c_str());
                return false;
            }
        }
        else
            prop.assign(svar.c_str(), value_str.c_str(), policy, fd, lt);
            
        if(cmd == PC_INC_MOD)
        {
            if(ftModValue != ftUnknown)
            {
                REProperty *rp = context.getProperty(mod_str);
                if(!rp) {
                  smsc_log_error(logger, "mod parameter '%s' not found in action context", mod_str.c_str());
                  return false;
                }
                md = rp->getInt();
            }
            else {
              md = mod;
            }
        }
    }

	PersClient& pc = PersClient::Instance();
	try
	{
		switch(cmd)
		{
			case PC_GET: pc.GetPropertyPrepare(profile, pk, svar.c_str(), sb); break;
			case PC_SET: pc.SetPropertyPrepare(profile, pk, prop, sb); break;
			case PC_DEL: pc.DelPropertyPrepare(profile, pk, svar.c_str(), sb); break;		
			case PC_INC: pc.IncPropertyPrepare(profile, pk, prop, sb); break;		
			case PC_INC_MOD: pc.IncModPropertyPrepare(profile, pk, prop, md, sb); break;		
		}
	}
	catch(PersClientException& e)		
	{
		smsc_log_error(logger, "PersClientException: batchPrepare: cmd=%s profile=%d var=%s, reason: %s", getStrCmd(), profile, var.c_str(), e.what());
		return false;
	}
    return true;
}

int PersAction::batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode)
{
    smsc_log_debug(logger,"ContinueRunning BatchAction 'PersAction cmd=%s. var=%s'...", getStrCmd(), var.c_str());
	PersClient& pc = PersClient::Instance();

    REProperty *statusProp = context.getProperty(status_str);
    if (statusProp) {
      statusProp->setInt(0);
    }
    REProperty *msgProp = context.getProperty(msg_str);
    if (msgProp) {
      msgProp->setStr("Ok");
    }
	try{

		switch(cmd)
		{
			case PC_GET:
			{
				Property prop;
				pc.GetPropertyResult(prop, sb);
		        REProperty *rep = context.getProperty(value_str);
		        setREPropFromPersProp(*rep, prop);
				break;
			}
			case PC_SET: pc.SetPropertyResult(sb); break;
			case PC_DEL: 
				//if(!pc.DelPropertyResult(sb))
	        	  //  smsc_log_warn(logger, "PersClientException: Property not found on deletion: %s", var.c_str());
                pc.DelPropertyResult(sb); break;		
			case PC_INC: 
			case PC_INC_MOD:
				uint32_t i = pc.IncModPropertyResult(sb);
		        REProperty *rep = context.getProperty(result_str);
		        if(rep) rep->setInt(i);
				break;		
		}
        return 0;
	}
	catch(const PersClientException& e)		
	{
      if (statusProp) {
        statusProp->setInt(e.getType());
      }
      if (msgProp) {
        msgProp->setStr(e.what());
      }
      smsc_log_debug(logger, "batchResult: cmd=%s profile=%d (skey=%s ikey=%d) var=%s, error code=%d : %s",
                     getStrCmd(), profile, context.getCommandProperty().abonentAddr.toString().c_str(),
                     getKey(context.getCommandProperty(), profile), var.c_str(), e.getType(), e.what());
		//throw SCAGException("PersClientException: batchResult: cmd=%s profile=%d (skey=%s ikey=%d) var=%s, reason: %s", getStrCmd(), profile, context.getCommandProperty().abonentAddr.toString().c_str(), getKey(context.getCommandProperty(), profile), var.c_str(), e.what());
      if (transactMode) {
        throw;
      }
      return e.getType();
	}
}

bool PersAction::RunBeforePostpone(ActionContext& context)
{
	time_t fd = final_date, lt = life_time;
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s. var=%s'...", getStrCmd(), var.c_str());

    CommandProperty& cp = context.getCommandProperty();
    auto_ptr<PersCallParams> params(new PersCallParams());

    REProperty *p;
	if(varType != ftUnknown && !(p = context.getProperty(var)))
	{
		smsc_log_error(logger, "Invalid var property  %s", var.c_str());
		return false;
	}
	const std::string& svar = varType == ftUnknown ? var : p->getStr();
    
    context.getSession().getLongCallContext().callCommandId = cmdToLongCallCmd(cmd);
    params->pt = profile;
    params->propName = svar;
    if (has_optional_key) {
      if (ftOptionalKey != PT_UNKNOWN) {
        REProperty *rp = context.getProperty(optional_key_str);
        if(!rp) {
          smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                          OPTIONAL_KEY, optional_key_str.c_str());
          return false;
        }
        if (profile == PT_ABONENT) {
          try {
            optional_skey = getAbntAddress(rp->getStr());
          } catch(const std::runtime_error& e) {
            smsc_log_error(logger, "'%s' parameter has error abonent profile key: %s", OPTIONAL_KEY, e.what());
            return false;
          }
        } else {
          optional_ikey = rp->getInt();
        }
      }
      params->skey = optional_skey;
      params->ikey = optional_ikey;
    } else {
      if(params->pt == PT_ABONENT)
          params->skey = cp.abonentAddr.toString();
      else
          params->ikey = getKey(cp, profile);
    }

	if(lifeTimeFieldType != ftUnknown)
	{
        REProperty *rp = context.getProperty(sLifeTime);
        if(!rp || !(lt = parseLifeTime(rp->getStr())))
		{
			smsc_log_error(logger, "Invalid lifeTime parameter %s(%s)", sLifeTime.c_str(), rp ? rp->getStr().c_str() : "");
			return false;
		}
	}
	if(finalDateFieldType != ftUnknown)
	{
        REProperty *rp = context.getProperty(sFinalDate);
        if(!rp || !(fd = parseFinalDate(rp->getStr())))
		{
			smsc_log_error(logger, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
			return false;
		}
	}
		
    if(cmd == PC_INC || cmd == PC_INC_MOD || cmd == PC_SET)
    {
        if(ftValue != ftUnknown)
        {
            REProperty *rp = context.getProperty(value_str);
            if(rp)
            {
                setPersPropFromREProp(params->prop, *rp);
                params->prop.setName(svar);
                params->prop.setTimePolicy(policy, fd, lt);
            }
            else
                return false;
        }
        else
            params->prop.assign(svar.c_str(), value_str.c_str(), policy, fd, lt);
            
        if(cmd == PC_INC_MOD)
        {
            if(ftModValue != ftUnknown)
            {
                REProperty *rp = context.getProperty(mod_str);
                if(!rp)
                    return false;
                params->mod = rp->getInt();
            }
            else {
              params->mod = mod;
            }
        }
    }
    
    context.getSession().getLongCallContext().setParams(params.release());
    return true;
}

void PersAction::ContinueRunning(ActionContext& context)
{
    smsc_log_debug(logger,"ContinueRunning Action 'PersAction cmd=%s. var=%s'...",
                    getStrCmd(), var.c_str());

    PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    REProperty *statusProp = context.getProperty(status_str);
    if (statusProp) {
      statusProp->setInt(params->error);
    }
    REProperty *msgProp = context.getProperty(msg_str);
    if (msgProp) {
      msgProp->setStr(params->error == 0 ? "Ok" : params->exception);
    }
    if(params->error != 0)
    {
        if(params->error == PROPERTY_NOT_FOUND && cmd == PC_GET)
        {
          REProperty *rep = context.getProperty(value_str);
          rep->setStr("");
        }
        smsc_log_debug(logger, "ContinueRunning: cmd=%s profile=%d (skey=%s ikey=%d) var=%s, error code=%d : %s",
                       getStrCmd(), profile, context.getCommandProperty().abonentAddr.toString().c_str(), getKey(context.getCommandProperty(), profile), var.c_str(), params->error, params->exception.c_str());
        return;
  		//throw SCAGException("PersClientException: continueRunning: cmd=%s profile=%d (skey=%s ikey=%d) var=%s, reason: %s", getStrCmd(), profile, context.getCommandProperty().abonentAddr.toString().c_str(), getKey(context.getCommandProperty(), profile), var.c_str(), params->exception.c_str());
    }
    if(cmd == PC_INC_MOD || cmd == PC_INC)
    {
        REProperty *rep = context.getProperty(result_str);
        if (rep) {
          rep->setInt(params->result);
        }
    }
    else if(cmd == PC_GET)
    {
        REProperty *rep = context.getProperty(value_str);
        setREPropFromPersProp(*rep, params->prop);
    }
        
    context.getSession().getLongCallContext().freeParams();
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
