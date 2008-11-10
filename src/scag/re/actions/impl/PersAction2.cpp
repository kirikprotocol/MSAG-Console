#include "PersAction2.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "sms/sms.h"
  
namespace {

const char* OPTIONAL_KEY = "key";

const char* getStrCmd(scag2::pers::util::PersCmd cmd)
{
    switch(cmd)
    {
    case scag2::pers::util::PC_DEL:
        return "profile:del";
    case scag2::pers::util::PC_SET:
        return "profile:set";
    case scag2::pers::util::PC_GET:
        return "profile:get";
    case scag2::pers::util::PC_INC:
        return "profile:inc";
    case scag2::pers::util::PC_INC_MOD:
        return "profile:inc-mod";
    case scag2::pers::util::PC_MTBATCH:
    case scag2::pers::util::PC_BATCH:
        return "profile:batch";
    default:
        return "unknown";
    };
}

static uint32_t cmdToLongCallCmd(uint32_t c)
{
    switch(c)
    {
    case scag2::pers::util::PC_DEL:     return scag2::lcm::PERS_DEL;
    case scag2::pers::util::PC_SET:     return scag2::lcm::PERS_SET;
    case scag2::pers::util::PC_GET:     return scag2::lcm::PERS_GET;
    case scag2::pers::util::PC_INC:     return scag2::lcm::PERS_INC;
    case scag2::pers::util::PC_INC_MOD: return scag2::lcm::PERS_INC_MOD;
    }
    return 0;
}
}


namespace scag2 {
namespace re {
namespace actions { namespace reprop = util::properties;

typedef reprop::Property REProperty;
using namespace pers::util;

void PersActionCommand::init( const SectionParams& params, PropertyObject propertyObject ) 
{
    bool bExist = false;
    std::string temp;
    ftVar = CheckParameter(params, propertyObject, name(), "var", true, true, temp, bExist);
    var.assign( temp.data(), temp.size() );

    /////////////////////////////////////////////
    bool statusExist = false;
    CheckParameter(params, propertyObject, name(), "status", false, false,
                   status, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, name(), "msg", false, false,
                   msg, msgExist);

    //////////////////////////////////////////////
    if (cmd == PC_DEL)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", getStrCmd(cmd), var.c_str());
        return;
    }

    if(cmd == PC_INC_MOD || cmd == PC_INC) {
        sValue = params.Exists("inc") ? params["inc"] : "1";
        bool resultExist = false;
        CheckParameter(params, propertyObject, name(), "result", false, false, sResult, resultExist);
    }
    else
    {
        if(!params.Exists("value"))
            throw SCAGException("PersAction 'value' : missing '%s' parameter", getStrCmd(cmd));
        sValue = params["value"];
    }

    const char* nm = 0;
    ftValue = ActionContext::Separate(sValue, nm);
    if(cmd == PC_GET && (ftValue == ftUnknown || ftValue == ftConst))
        throw InvalidPropertyException("PersAction '%s': 'value' parameter should be an lvalue. Got %s", getStrCmd(cmd), sValue.c_str());

    if (ftValue == ftField) 
    {
        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, nm, propertyObject.transport);
        if (!(at & atRead) || (cmd == PC_GET && !(at & atWrite))) 
            throw InvalidPropertyException("PersAction '%s': cannot read/modify property '%s' - no access", sValue.c_str());
    }

    if (cmd == PC_GET)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", getStrCmd(cmd), var.c_str());
        return;
    }

    if(cmd == PC_INC_MOD)
    {
        mod = 0;
        sMod = params.Exists("mod") ? params["mod"] : "0";

        ftModValue = ActionContext::Separate(sMod, nm); 
        if(ftModValue == ftField) 
        {
            AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, nm, propertyObject.transport);
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

        ftFinalDate = CheckParameter(params, propertyObject, name(), "finaldate", true, true, sFinalDate, bExist);
        if(ftFinalDate == ftUnknown)
        {
            finalDate = parseFinalDate(sFinalDate.c_str());
            if(!finalDate)
                throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", getStrCmd(cmd));
        }
        
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, final_date=%d", getStrCmd(cmd), var.c_str(), policy, finalDate);
        return;
    }

    lifetime_.init(params, propertyObject, name(), "lifetime", true, true);
    /*
    if(ftLifeTime == ftUnknown)
    {
        lifeTime = parseLifeTime(sLifeTime.c_str());
        if(!lifeTime)
            throw SCAGException("PersAction '%s' : invalid 'lifetime' parameter", getStrCmd(cmd));
     }
     */
    smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, mod=%d",
                   getStrCmd(cmd), var.c_str(), policy, mod);
}

    
int PersActionCommand::fillSB( ActionContext& context, SerialBuffer& sb )
{
    assert( sb.getPos() != 0 );

    // --- get property name
    REProperty *p;
    if (ftVar != ftUnknown && !(p = context.getProperty(var.c_str()))) {
        smsc_log_error(logger, "Invalid var property  %s", var.c_str());
        // FIXME: ask Vitaly if I could use PROPNOTFOUND for RE prop
        return pers::util::PROPERTY_NOT_FOUND;
    }
    const REProperty::string_type& svar = ftVar == ftUnknown ? var : p->getStr();
    if ( cmd == PC_DEL || cmd == PC_GET ) {
        sb.WriteString( svar.c_str() );
        return 0;
    } else if ( cmd != PC_SET && cmd != PC_INC && cmd != PC_INC_MOD ) {
        smsc_log_error(logger, "Invalid command %d", int(cmd) );
        return pers::util::COMMAND_NOTSUPPORT;
    }

    // --- time policy
    time_t fd = finalDate;
    uint32_t lt = lifetime_.getTime( name(), context );
    if (ftFinalDate != ftUnknown)  {
        REProperty *rp = context.getProperty(sFinalDate);
        if (!rp || !(fd = parseFinalDate(rp->getStr().c_str()))) {
            smsc_log_error(logger, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
            return pers::util::PROPERTY_NOT_FOUND;
        }
    }

    pers::util::Property prop;
    if (ftValue != ftUnknown)
    {
        REProperty *rp = context.getProperty(sValue);
        if (!rp) return pers::util::PROPERTY_NOT_FOUND;

        switch ( rp->getType() ) {
        case (reprop::pt_int) :
            prop.setIntValue(rp->getInt()); break;
        case (reprop::pt_bool) :
            prop.setBoolValue(rp->getBool()); break;
        case (reprop::pt_date) :
            prop.setDateValue(rp->getDate()); break;
        case (reprop::pt_str) :
            prop.setStringValue(rp->getStr().c_str()); break;
        default :
            smsc_log_error( logger, "Wrong property type %d", int(rp->getType()) );
            return pers::util::INVALID_PROPERTY_TYPE;
        }

    } else {
        prop.setStringValue( sValue.c_str() );
    }
    prop.setName( svar.c_str() );
    prop.setTimePolicy( policy, fd, lt );

    // modulus
    if ( cmd == PC_INC_MOD ) {
        uint32_t realmod = mod;
        if (ftModValue != ftUnknown) {
            REProperty *rp = context.getProperty(sMod);
            if (!rp) {
                return pers::util::PROPERTY_NOT_FOUND;
            }
            realmod = static_cast<uint32_t>(rp->getInt());
        }
        sb.WriteInt32( realmod );
    }
    prop.Serialize( sb );
    return 0;
}


int PersActionCommand::readSB( ActionContext& context, SerialBuffer& sb )
{
    int result = 0;
    switch (cmd) {
    case (PC_DEL) :
    case (PC_SET) :
        break;
    case (PC_INC) :
    case (PC_INC_MOD) : {
        uint32_t i = sb.ReadInt32();
        REProperty* rp = context.getProperty(sResult);
        if (rp) rp->setInt(i);
        break;
    }
    case (PC_GET) : {
        pers::util::Property prop;
        prop.Deserialize( sb );
        REProperty* rp = context.getProperty( sValue );
        if (rp) {
            switch (prop.getType()) {
            case (INT) :
                rp->setInt(prop.getIntValue());
                break;
            case (BOOL) :
                rp->setBool(prop.getBoolValue());
                break;
            case (DATE) :
                rp->setDate(prop.getDateValue());
                break;
            case (STRING) :
                rp->setStr(prop.getStringValue().c_str());
                break;
            default :
                result = pers::util::INVALID_PROPERTY_TYPE;
            }
        }
    }
    default :
        result = pers::util::COMMAND_NOTSUPPORT;
    }
    if ( result ) setStatus(context,result);
    return result;
}


IParserHandler * PersActionCommand::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'profile:': cannot have a child object");
}


bool PersActionCommand::FinishXMLSubSection(const std::string& name)
{
    return true;
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


time_t PersActionCommand::parseFinalDate(const char* s)
{
    struct tm time;
    char *ptr;

    ptr = strptime(s, "%d.%m.%Y %T", &time);
    if(!ptr || *ptr) return 0;

    return mktime(&time);
}


// =========================================================================


void PersActionBase::init(const SectionParams& params, PropertyObject propertyObject)
{
    if (!params.Exists("type") || (profile = getProfileTypeFromStr(params["type"])) == PT_UNKNOWN)
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", getStrCmd(cmd));
    
    ftOptionalKey = CheckParameter( params,propertyObject, "persaction", OPTIONAL_KEY, false, true,
                                    optionalKeyStr, hasOptionalKey );
    if (hasOptionalKey && ftOptionalKey == ftUnknown) {
        smsc_log_debug(logger, "PersAction: optional_key_str=%s, optional key exists",
                       optionalKeyStr.c_str(), hasOptionalKey);

        if (profile == PT_ABONENT) {
            try {
                optionalKeyStr = getAbntAddress(optionalKeyStr.c_str());
            } catch(const std::runtime_error& e) {
                throw SCAGException("PersAction '%s' : '%s' parameter has error abonent profile key: %s",
                                    getStrCmd(cmd), OPTIONAL_KEY, e.what());
            }
        } else if (strcmp(optionalKeyStr.c_str(), "0") && !(optionalKeyInt = atoi(optionalKeyStr.c_str()))) {
            throw SCAGException("PersAction '%s' : '%s' parameter not a number in not abonent profile type. key=%s",
                                getStrCmd(cmd), OPTIONAL_KEY, optionalKeyStr.c_str());
        }
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


std::string PersActionBase::getAbntAddress(const char* _address) {
    Address address(_address);
    if (_address[0] != '.') {
        address.setNumberingPlan(1);
        address.setTypeOfNumber(1);
    }
    return address.toString();
}


bool PersActionBase::setKey( ActionContext& context, PersCallParams* params )
{
    if ( !hasOptionalKey ) {
        CommandProperty& cp = context.getCommandProperty();
        switch (profile) {
        case (PT_ABONENT) : params->setKey( cp.abonentAddr.toString() ); break;
        case (PT_SERVICE) : params->setKey( cp.serviceId ); break;
        case (PT_OPERATOR) : params->setKey( cp.operatorId ); break;
        case (PT_PROVIDER) : params->setKey( cp.providerId ); break;
        default :
            params->setStatus( context, pers::util::BAD_REQUEST );
            return false;
        }
        return true;
    }

    if ( ftOptionalKey != ftUnknown ) {
        REProperty *rp = context.getProperty(optionalKeyStr);
        if(!rp) {
            smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                           OPTIONAL_KEY, optionalKeyStr.c_str());
            params->setStatus(context,pers::util::PROPERTY_NOT_FOUND);
            return false;
        }
        if (profile == PT_ABONENT) {
            try {
                params->setKey( getAbntAddress(rp->getStr().c_str()) );
            } catch(const std::runtime_error& e) {
                smsc_log_error(logger, "'%s' parameter has error abonent profile key: %s",
                               OPTIONAL_KEY, e.what());
                params->setStatus(context,pers::util::INVALID_KEY);
                return false;
            }
        } else {
            params->setKey( static_cast<uint32_t>(rp->getInt()) );
        }
    } else if ( profile == PT_ABONENT ) {
        params->setKey( optionalKeyStr );
    } else {
        params->setKey( optionalKeyInt );
    }
    return true;
}


// =========================================================================


void PersAction::init(const SectionParams& params, PropertyObject propertyObject) 
{
    PersActionBase::init(params,propertyObject);
    persCommand.init(params,propertyObject);
}


bool PersAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s, profile=%d var=%s'...", getStrCmd(cmd), profile, persCommand.propertyName());

    if ( ! persCommand.canProcessRequest(context) ) return false;

    auto_ptr<PersCallParams> params(new PersCallParams(profile, persCommand.statusName(), persCommand.msgName()));
    if ( !setKey(context, params.get()) ) {
        return false;
    }
    params->single( persCommand );
    if ( ! params->fillSB( context ) ) return false;

    context.getSession().getLongCallContext().callCommandId = cmdToLongCallCmd(cmd);
    context.getSession().getLongCallContext().setParams(params.release());
    return true;
}


void PersAction::ContinueRunning(ActionContext& context)
{
    PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    smsc_log_debug(logger, "ContinueRunning: cmd=%s (skey=%s ikey=%d) var=%s",
                   getStrCmd(cmd),
                   params->proxy().getKey().skey,
                   params->proxy().getKey().ikey,
                   persCommand.propertyName() );
    params->readSB( context );
    // setStatus(context, params->error, persCommand.getStatus(), persCommand.getMsg());
    // persCommand.ContinueRunning(context);
}


IParserHandler * PersAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'profile:': cannot have a child object");
}


bool PersAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}


#if 0

typedef util::properties::Property REProperty;

using namespace scag::stat;
const char* OPTIONAL_KEY = "key";
const size_t ACTION_IDX_BUFFER_SIZE = 10;

char buf; // crashs without this :)

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

void PersActionBase::getOptionalKey(const std::string& key_str) {
  if (profile == PT_ABONENT) {
    try {
      optionalSkey = getAbntAddress(key_str.c_str());
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
    std::string temp;
  ftVar = CheckParameter(params, propertyObject, name(), "var", true, true, temp, bExist);
    var.assign( temp.data(), temp.size() );
/////////////////////////////////////////////
    bool statusExist = false;
    CheckParameter(params, propertyObject, name(), "status", false, false,
                    status, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, name(), "msg", false, false,
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
    CheckParameter(params, propertyObject, name(), "result", false, false, sResult, resultExist);
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

      ftFinalDate = CheckParameter(params, propertyObject, name(), "finaldate", true, true, sFinalDate, bExist);
      if(ftFinalDate == ftUnknown)
      {
          finalDate = parseFinalDate(sFinalDate.c_str());
          if(!finalDate)
              throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", getStrCmd(cmd));
      }

      smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, final_date=%d", getStrCmd(cmd), var.c_str(), policy, finalDate);
      return;
  }

  ftLifeTime = CheckParameter(params, propertyObject, name(), "lifetime", true, true, sLifeTime, bExist);
  if(ftLifeTime == ftUnknown)
  {
      lifeTime = parseLifeTime(sLifeTime.c_str());
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
    
    ftOptionalKey = CheckParameter(params,propertyObject, name(), OPTIONAL_KEY, false, true,
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
    namespace reprop = util::properties;

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
            rep.setStr(prop.getStringValue().c_str());
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

std::string PersActionBase::getAbntAddress(const char* _address) {
  Address address(_address);
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
  if (ftVar != ftUnknown && !(p = context.getProperty(var.c_str()))) {
    smsc_log_error(logger, "Invalid var property  %s", var.c_str());
    return false;
  }
  const REProperty::string_type& svar = ftVar == ftUnknown ? var : p->getStr();
  time_t fd = finalDate;
  uint32_t lt = lifeTime;
  params->propName.assign(svar.data(),svar.size());
  if (ftLifeTime != ftUnknown) {
    REProperty *rp = context.getProperty(sLifeTime);
    if (!rp || !(lt = parseLifeTime(rp->getStr().c_str()))) {
      smsc_log_error(logger, "Invalid lifeTime parameter %s(%s)", sLifeTime.c_str(), rp ? rp->getStr().c_str() : "");
      return false;
    }
  }
  if (ftFinalDate != ftUnknown)  {
    REProperty *rp = context.getProperty(sFinalDate);
    if (!rp || !(fd = parseFinalDate(rp->getStr().c_str()))) {
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
    params->prop.setName(svar.c_str());
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

bool PersActionBase::canProcessRequest(ActionContext& context, const string& statusName, const string& msgName) {
  int status = PersClient::Instance().getClientStatus();
  if (status > 0) {
    smsc_log_warn(logger,"Run Action 'PersAction: Pers Client %s'", scag::pers::util::strs[status]);
    setStatus(context, status, statusName, msgName);
    return false;
  }
  return true;
}

bool PersAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s, profile=%d var=%s'...", getStrCmd(cmd), profile, persCommand.getVar());

    if (!canProcessRequest(context, persCommand.getStatus(), persCommand.getMsg())) {
      return false;
    }

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

void PersActionBase::setStatus(ActionContext& context, int status, const string& statusName, const string& msgName, int actionIdx) {
  string msg = "Ok";
  if (status != 0) {
    msg = scag::pers::util::strs[status];
    if (actionIdx > 0) {
      char idx_buffer[ACTION_IDX_BUFFER_SIZE];
      int n = snprintf(idx_buffer, ACTION_IDX_BUFFER_SIZE, "%d", actionIdx);
      msg += " in action ";
      msg.append(idx_buffer, n);
    }
  }
  REProperty *statusProp = context.getProperty(statusName);
  if (statusProp) {
    statusProp->setInt(status);
  }
  REProperty *msgProp = context.getProperty(msgName);
  if (msgProp) {
    msgProp->setStr(msg.c_str());
  }

}

void PersActionCommand::ContinueRunning(ActionContext& context) {
  PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
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
    PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    setStatus(context, params->error, persCommand.getStatus(), persCommand.getMsg());
    persCommand.ContinueRunning(context);
}

#endif

}//actions
}//re
}//scag2
