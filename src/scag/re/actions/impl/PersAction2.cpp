#include "PersAction2.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/base/CommandAdapter2.h"
#include "sms/sms.h"
  
namespace scag2 { namespace re { namespace actions {

typedef scag::util::properties::Property REProperty;

using namespace scag::stat;
const char* OPTIONAL_KEY = "key";

char buf; // crashs without this :)

static const char* getStrCmd(PersCmd cmd)
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
        case PC_MTBATCH:
        case PC_BATCH:
            return "profile:batch";
        default:
            return "unknown";
    }
}

ProfileType PersActionBase::getProfileTypeFromStr(const std::string& str)
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

TimePolicy PersActionCommand::getPolicyFromStr(const std::string& str)
{
    if(!strcmp(str.c_str(), "INFINIT"))
        return scag::pers::util::INFINIT;
    else if(!strcmp(str.c_str(), "FIXED"))
        return scag::pers::util::FIXED;
    else if(!strcmp(str.c_str(), "ACCESS"))
        return scag::pers::util::ACCESS;
    else if(!strcmp(str.c_str(), "R_ACCESS"))
        return scag::pers::util::R_ACCESS;
    else if(!strcmp(str.c_str(), "W_ACCESS"))
        return scag::pers::util::W_ACCESS;
    else
        return scag::pers::util::UNKNOWN;
}

time_t PersActionCommand::parseFinalDate(const std::string& s)
{
	struct tm time;
    char *ptr;

    ptr = strptime(s.c_str(), "%d.%m.%Y %T", &time);
    if(!ptr || *ptr)
		return 0;

    return mktime(&time);
}

uint32_t PersActionCommand::parseLifeTime(const std::string& s)
{
	uint32_t hour = 0, min = 0, sec = 0;
    if(sscanf(s.c_str(), "%u:%u:%u", &hour, &min, &sec) < 3)
		return 0;

    return hour * 3600 + min * 60 + sec;
}

void PersActionBase::getOptionalKey(const std::string& key_str) {
  if (profile == PT_ABONENT) {
    try {
      optionalSkey = getAbntAddress(key_str);
    } catch(const std::runtime_error& e) {
        throw SCAGException("PersAction '%s' : '%s' parameter has error abonent profile key: %s",
                            getStrCmd(cmd), OPTIONAL_KEY, e.what());
    }
  } else if (strcmp(key_str.c_str(), "0") && !(optionalIkey = atoi(key_str.c_str()))) {
      throw SCAGException("PersAction '%s' : '%s' parameter not a number in not abonent profile type. key=%s",
                           getStrCmd(cmd), OPTIONAL_KEY, key_str.c_str());
  }
}

void PersActionCommand::init(const SectionParams& params, PropertyObject propertyObject) {
  bool bExist = false;
  const char* name = 0;
  ftVar = CheckParameter(params, propertyObject, "PersAction", "var", true, true, var, bExist);
/////////////////////////////////////////////
    bool statusExist = false;
    CheckParameter(params, propertyObject, "PersAction", "status", false, false,
                    status, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, "PersAction", "msg", false, false,
                    msg, msgExist);
//////////////////////////////////////////////
  if(cmd == PC_DEL)
  {
      smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", getStrCmd(cmd), var.c_str());
      return;
  }

  if(cmd == PC_INC_MOD || cmd == PC_INC) {
    sValue = params.Exists("inc") ? params["inc"] : "1";
    bool resultExist = false;
    CheckParameter(params, propertyObject, "PersAction", "result", false, false, sResult, resultExist);
  }
  else
  {
      if(!params.Exists("value"))
          throw SCAGException("PersAction 'value' : missing '%s' parameter", getStrCmd(cmd));
      sValue = params["value"];
  }

  ftValue = ActionContext::Separate(sValue, name); 
  if(cmd == PC_GET && (ftValue == ftUnknown || ftValue == ftConst))
      throw InvalidPropertyException("PersAction '%s': 'value' parameter should be an lvalue. Got %s", getStrCmd(cmd), sValue.c_str());

  if(ftValue == ftField) 
  {
      AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);
      if (!(at & atRead) || (cmd == PC_GET && !(at & atWrite))) 
          throw InvalidPropertyException("PersAction '%s': cannot read/modify property '%s' - no access", sValue.c_str());
  }

  if(cmd == PC_GET)
  {
      smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", getStrCmd(cmd), var.c_str());
      return;
  }

  if(cmd == PC_INC_MOD)
  {
      mod = 0;
      sMod = params.Exists("mod") ? params["mod"] : "0";

      ftModValue = ActionContext::Separate(sMod, name); 
      if(ftModValue == ftField) 
      {
          AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);
          if(!(at & atRead)) 
              throw InvalidPropertyException("PersAction '%s': cannot read property '%s' - no access", sMod.c_str());
      }

      if(ftModValue == ftUnknown && strcmp(sMod.c_str(), "0") && !(mod = atoi(sMod.c_str())))
          throw SCAGException("PersAction '%s' : 'mod' parameter not a number. mod=%s", getStrCmd(cmd), sMod.c_str());
  }

  if(!params.Exists("policy") || (policy = getPolicyFromStr(params["policy"])) == scag::pers::util::UNKNOWN)
      throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", getStrCmd(cmd));

  if(policy == INFINIT)
  {
      smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, mod=%d", getStrCmd(cmd), var.c_str(), policy, mod);
      return;
  }

  if(policy == FIXED && params.Exists("finaldate"))
  {

      ftFinalDate = CheckParameter(params, propertyObject, "PersAction", "finaldate", true, true, sFinalDate, bExist);
      if(ftFinalDate == ftUnknown)
      {
          finalDate = parseFinalDate(sFinalDate);
          if(!finalDate)
              throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", getStrCmd(cmd));
      }

      smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, final_date=%d", getStrCmd(cmd), var.c_str(), policy, finalDate);
      return;
  }

  ftLifeTime = CheckParameter(params, propertyObject, "PersAction", "lifetime", true, true, sLifeTime, bExist);
  if(ftLifeTime == ftUnknown)
  {
      lifeTime = parseLifeTime(sLifeTime);
      if(!lifeTime)
          throw SCAGException("PersAction '%s' : invalid 'lifetime' parameter", getStrCmd(cmd));
  }

  smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, life_time=%d, mod=%d",
                  getStrCmd(cmd), var.c_str(), policy, lifeTime, mod);
}

void PersActionBase::init(const SectionParams& params, PropertyObject propertyObject)
{
	bool bExist;
    const char * name = 0;

    if(!params.Exists("type") || (profile = getProfileTypeFromStr(params["type"])) == PT_UNKNOWN) 
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", getStrCmd(cmd));
    
    ftOptionalKey = CheckParameter(params,propertyObject, "PersAction", OPTIONAL_KEY, false, true,
                                       optionalKeyStr, hasOptionalKey);
    if (hasOptionalKey && ftOptionalKey == ftUnknown) {
      smsc_log_debug(logger, "PersAction: optional_key_str=%s, optional key exists",
                      optionalKeyStr.c_str(), hasOptionalKey);
      getOptionalKey(optionalKeyStr);
    }
}

void PersAction::init(const SectionParams& params, PropertyObject propertyObject) {
  PersActionBase::init(params,propertyObject);
  persCommand.init(params,propertyObject);
}

uint32_t getKey(const CommandProperty& cp, ProfileType pt)
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

static void setPersPropFromREProp(scag::pers::util::Property& prop, REProperty& rep)
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
            break;
    }
}

static void setREPropFromPersProp(REProperty& rep, scag::pers::util::Property& prop)
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

std::string PersActionBase::getAbntAddress(const std::string& _address) {
  Address address(_address.c_str());
  if (_address[0] != '.') {
    address.setNumberingPlan(1);
    address.setTypeOfNumber(1);
  }
  return address.toString();
}

int PersActionCommand::batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode)
{
    smsc_log_debug(logger,"ContinueRunning BatchAction 'PersAction cmd=%s. var=%s'...", getStrCmd(cmd), var.c_str());
	PersClient& pc = PersClient::Instance();

    REProperty *statusProp = context.getProperty(status);
    if (statusProp) {
      statusProp->setInt(0);
    }
    REProperty *msgProp = context.getProperty(msg);
    if (msgProp) {
      msgProp->setStr("Ok");
    }
	try{
		switch(cmd)
		{
			case PC_GET:
			{
				scag::pers::util::Property prop;
				pc.GetPropertyResult(prop, sb);
		        REProperty *rep = context.getProperty(sValue);
		        setREPropFromPersProp(*rep, prop);
				break;
			}
			case PC_SET: pc.SetPropertyResult(sb); break;
			case PC_DEL: pc.DelPropertyResult(sb); break;		
			case PC_INC: 
			case PC_INC_MOD:
				uint32_t i = pc.IncModPropertyResult(sb);
		        REProperty *rep = context.getProperty(sResult);
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
      smsc_log_debug(logger, "batchResult: cmd=%s var=%s, error code=%d : %s",
                     getStrCmd(cmd), var.c_str(), e.getType(), e.what());
      if (transactMode) {
        throw;
      }
      return e.getType();
	}
}

bool PersActionCommand::batchPrepare(ActionContext& context, SerialBuffer& sb)
{
    smsc_log_debug(logger,"Run BatchPrepare 'PersAction cmd=%s. var=%s'...", getStrCmd(cmd), var.c_str());
    PersCallParams params;
    if (!RunBeforePostpone(context, &params)) {
      return false;
    }

    PersClient& pc = PersClient::Instance();
    try
    {
        switch(cmd)
        {
          
            case PC_GET: pc.GetPropertyPrepare(params.propName.c_str(), sb); break;
            case PC_SET: pc.SetPropertyPrepare(params.prop, sb); break;
            case PC_DEL: pc.DelPropertyPrepare(params.propName.c_str(), sb); break;		
            case PC_INC: pc.IncPropertyPrepare(params.prop, sb); break;		
            case PC_INC_MOD: pc.IncModPropertyPrepare(params.prop, params.mod, sb); break;		
        }
        return true;
    }
    catch(PersClientException& e)		
    {
        smsc_log_error(logger, "PersClientException: batchPrepare: cmd=%s var=%s, reason: %s", getStrCmd(cmd), var.c_str(), e.what());
        return false;
    }
}

bool PersActionCommand::RunBeforePostpone(ActionContext& context, PersCallParams* params) {
  REProperty *p;
  if (ftVar != ftUnknown && !(p = context.getProperty(var))) {
    smsc_log_error(logger, "Invalid var property  %s", var.c_str());
    return false;
  }
  const std::string& svar = ftVar == ftUnknown ? var : p->getStr();
  time_t fd = finalDate;
  uint32_t lt = lifeTime;
  params->propName = svar;
  if (ftLifeTime != ftUnknown) {
    REProperty *rp = context.getProperty(sLifeTime);
    if (!rp || !(lt = parseLifeTime(rp->getStr()))) {
      smsc_log_error(logger, "Invalid lifeTime parameter %s(%s)", sLifeTime.c_str(), rp ? rp->getStr().c_str() : "");
      return false;
    }
  }
  if (ftFinalDate != ftUnknown)  {
    REProperty *rp = context.getProperty(sFinalDate);
    if (!rp || !(fd = parseFinalDate(rp->getStr()))) {
      smsc_log_error(logger, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
      return false;
    }
  }
  if (cmd != PC_INC && cmd != PC_INC_MOD && cmd != PC_SET) {
    return true;
  }

  if (ftValue != ftUnknown)
  {
    REProperty *rp = context.getProperty(sValue);
    if (!rp) {
      return false;
    }
    setPersPropFromREProp(params->prop, *rp);
    params->prop.setName(svar);
    params->prop.setTimePolicy(policy, fd, lt);
  } else {
    params->prop.assign(svar.c_str(), sValue.c_str(), policy, fd, lt);
  }

  if (cmd != PC_INC_MOD) {
    return true;
  }

  if (ftModValue != ftUnknown) {
    REProperty *rp = context.getProperty(sMod);
    if (!rp) {
      return false;
    }
    params->mod = static_cast<uint32_t>(rp->getInt());
  }
  else {
    params->mod = mod;
  }
  return true;
}

const char* PersActionCommand::getVar() {
  return var.c_str();
}

bool PersActionBase::setKey(ActionContext& context, PersCallParams* params) {
  if (!hasOptionalKey) {
    CommandProperty& cp = context.getCommandProperty();
    if(params->pt == PT_ABONENT)
        params->skey = cp.abonentAddr.toString();
    else
        params->ikey = getKey(cp, profile);
    return true;
  }
  if (ftOptionalKey != ftUnknown) {
    REProperty *rp = context.getProperty(optionalKeyStr);
    if(!rp) {
      smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                      OPTIONAL_KEY, optionalKeyStr.c_str());
      return false;
    }
    if (params->pt == PT_ABONENT) {
      try {
        optionalSkey = getAbntAddress(rp->getStr());
      } catch(const std::runtime_error& e) {
        smsc_log_error(logger, "'%s' parameter has error abonent profile key: %s", OPTIONAL_KEY, e.what());
        return false;
      }
    } else {
      optionalIkey = static_cast<uint32_t>(rp->getInt());
    }
  }
  params->skey = optionalSkey;
  params->ikey = optionalIkey;
  return true;
}

bool PersAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s, profile=%d var=%s'...", getStrCmd(cmd), profile, persCommand.getVar());

    auto_ptr<PersCallParams> params(new PersCallParams());
    params->pt = profile;

    if (!setKey(context, params.get())) {
      return false;
    }

    context.getSession().getLongCallContext().callCommandId = cmdToLongCallCmd(cmd);
    persCommand.RunBeforePostpone(context, params.get());
    context.getSession().getLongCallContext().setParams(params.release());
    return true;
}

void PersActionCommand::ContinueRunning(ActionContext& context) {
  PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
  REProperty *statusProp = context.getProperty(status);
  if (statusProp) {
    statusProp->setInt(params->error);
  }
  REProperty *msgProp = context.getProperty(msg);
  if (msgProp) {
    msgProp->setStr(params->error == 0 ? "Ok" : params->exception);
  }
  if (params->error != 0) {
    if(params->error == scag::pers::util::PROPERTY_NOT_FOUND && cmd == PC_GET) {
      REProperty *rep = context.getProperty(sValue);
      rep->setStr("");
    }
    smsc_log_debug(logger, "ContinueRunning: cmd=%s var=%s, error code=%d : %s",
                   getStrCmd(cmd), var.c_str(), params->error, params->exception.c_str());
    return;
  }
  if (cmd == PC_INC_MOD || cmd == PC_INC) {
    REProperty *rep = context.getProperty(sResult);
    if (rep) {
      rep->setInt(params->result);
    }
  } else if (cmd == PC_GET) {
    REProperty *rep = context.getProperty(sValue);
    setREPropFromPersProp(*rep, params->prop);
  }
  context.getSession().getLongCallContext().freeParams();
}

void PersAction::ContinueRunning(ActionContext& context)
{
    smsc_log_debug(logger, "ContinueRunning: cmd=%s (skey=%s ikey=%d) var=%s",
                   getStrCmd(cmd), context.getCommandProperty().abonentAddr.toString().c_str(), getKey(context.getCommandProperty(), profile), persCommand.getVar());
    persCommand.ContinueRunning(context);
}

IParserHandler * PersAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'profile:': cannot have a child object");
}

bool PersAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}

IParserHandler * PersActionCommand::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'profile:': cannot have a child object");
}

bool PersActionCommand::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}//actions
}//re
}//scag2
