#include "PersAction2.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/PersCallWrapper.h"
#include "scag/pvss/base/PersClient.h"
#include "scag/pvss/base/PersClientException.h"
#include "sms/sms.h"
  
namespace {

const char* OPTIONAL_KEY = "key";

static uint32_t cmdToLongCallCmd(uint32_t c)
{
    switch(c)
    {
    case scag2::pvss::PC_DEL:     return scag2::lcm::PERS_DEL;
    case scag2::pvss::PC_SET:     return scag2::lcm::PERS_SET;
    case scag2::pvss::PC_GET:     return scag2::lcm::PERS_GET;
    case scag2::pvss::PC_INC_RESULT: return scag2::lcm::PERS_INC;
    case scag2::pvss::PC_INC_MOD:    return scag2::lcm::PERS_INC_MOD;
    }
    return 0;
}
}


namespace scag2 {
namespace re {
namespace actions { namespace reprop = util::properties;

typedef reprop::Property REProperty;
using namespace pvss;


bool PersActionResultRetriever::canProcessRequest( ActionContext& ctx )
{
    int status = pvss::PersClient::Instance().getClientStatus();
    if ( status > 0 ) {
        setStatus( ctx, status );
        return false;
    }
    return true;
}


void PersActionResultRetriever::setStatus( ActionContext& context, int status, int actionIdx )
{
    __trace2__("pers: setting status %d, actionidx %d", status, actionIdx );

    REProperty *statusProp = context.getProperty(statusName());
    if (statusProp) {
        statusProp->setInt(status);
    }
    REProperty *msgProp = context.getProperty(msgName());
    while (msgProp) {
        if ( status < 0 ) status = pvss::UNKNOWN_EXCEPTION;
        const char* m = pvss::exceptionReasons[status];
        if (status != 0 && actionIdx > 0) {
            std::string msg = m;
            char idx_buffer[40];
            snprintf(idx_buffer, sizeof(idx_buffer), " in action %d", actionIdx);
            msg += idx_buffer;
            msgProp->setStr(msg.c_str());
            break;
        }
        msgProp->setStr(m);
        break;
    }
}


void PersActionResultRetriever::storeResults( const PersCommand& cmd, ActionContext& ctx )
{
    setStatus( ctx, cmd.status(), cmd.failIndex() );
}


// =================================================================================


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
    if (cmdType() == PC_DEL)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", pvss::persCmdName(cmdType()), var.c_str());
        return;
    }

    if (cmdType() == PC_INC_MOD || cmdType() == PC_INC_RESULT) {
        sValue = params.Exists("inc") ? params["inc"] : "1";
        bool resultExist = false;
        CheckParameter(params, propertyObject, name(), "result", false, false, sResult, resultExist);
    }
    else
    {
        if(!params.Exists("value"))
            throw SCAGException("PersAction 'value' : missing '%s' parameter", pvss::persCmdName(cmdType()));
        sValue = params["value"];
    }

    const char* nm = 0;
    ftValue = ActionContext::Separate(sValue, nm);
    if(cmdType() == PC_GET && (ftValue == ftUnknown || ftValue == ftConst))
        throw InvalidPropertyException("PersAction '%s': 'value' parameter should be an lvalue. Got %s", pvss::persCmdName(cmdType()), sValue.c_str());

    if (ftValue == ftField) 
    {
        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, nm, propertyObject.transport);
        if (!(at & atRead) || (cmdType() == PC_GET && !(at & atWrite))) 
            throw InvalidPropertyException("PersAction '%s': cannot read/modify property '%s' - no access", sValue.c_str());
    }

    if (cmdType() == PC_GET)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", pvss::persCmdName(cmdType()), var.c_str());
        return;
    }

    if (cmdType() == PC_INC_MOD)
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
            throw SCAGException("PersAction '%s' : 'mod' parameter not a number. mod=%s", pvss::persCmdName(cmdType()), sMod.c_str());
    }

    if(!params.Exists("policy") || (policy = getPolicyFromStr(params["policy"])) == scag::pvss::UNKNOWN)
        throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", pvss::persCmdName(cmdType()));

    if(policy == INFINIT)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, mod=%d", pvss::persCmdName(cmdType()), var.c_str(), policy, mod);
        return;
    }

    if(policy == FIXED && params.Exists("finaldate"))
    {

        ftFinalDate = CheckParameter(params, propertyObject, name(), "finaldate", true, true, sFinalDate, bExist);
        if(ftFinalDate == ftUnknown)
        {
            finalDate = parseFinalDate(sFinalDate.c_str());
            if(!finalDate)
                throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", pvss::persCmdName(cmdType()));
        }
        
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, final_date=%d", pvss::persCmdName(cmdType()), var.c_str(), policy, finalDate);
        return;
    }

    lifetime_.init(params, propertyObject, name(), "lifetime", true, true);
    /*
    if(ftLifeTime == ftUnknown)
    {
        lifeTime = parseLifeTime(sLifeTime.c_str());
        if(!lifeTime)
            throw SCAGException("PersAction '%s' : invalid 'lifetime' parameter", pvss::persCmdName(cmd));
     }
     */
    smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, mod=%d",
                   pvss::persCmdName(cmdType()), var.c_str(), policy, mod);
}

    
int PersActionCommand::fillCommand( ActionContext& context, pvss::PersCommandSingle& command )
{
    // --- get property name
    int stat = 0;
    do {

        REProperty *p;
        if (ftVar != ftUnknown && !(p = context.getProperty(var.c_str()))) {
            smsc_log_error(logger, "Invalid var property  %s", var.c_str());
            // FIXME: ask Vitaly if I could use PROPNOTFOUND for RE prop
            stat = pvss::PROPERTY_NOT_FOUND;
            break;
        }

        const REProperty::string_type& svar = ( ftVar == ftUnknown ? var : p->getStr() );
        pvss::Property& prop = command.property();
        prop.setName( svar.c_str() );
        if ( cmdType() == PC_DEL || cmdType() == PC_GET ) {
            break;
        } else if ( cmdType() != PC_SET && cmdType() != PC_INC_RESULT && cmdType() != PC_INC_MOD ) {
            smsc_log_error(logger, "Invalid command %d", int(cmdType()) );
            stat = pvss::COMMAND_NOTSUPPORT;
            break;
        }

        // --- time policy
        time_t fd = finalDate;
        uint32_t lt = lifetime_.getTime( name(), context );
        if (ftFinalDate != ftUnknown)  {
            REProperty *rp = context.getProperty(sFinalDate);
            if (!rp || !(fd = parseFinalDate(rp->getStr().c_str()))) {
                smsc_log_error(logger, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
                stat = pvss::PROPERTY_NOT_FOUND;
                break;
            }
        }

        if (ftValue != ftUnknown)
        {
            REProperty *rp = context.getProperty(sValue);
            if (!rp) {
                stat = pvss::PROPERTY_NOT_FOUND;
                break;
            }

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
                stat = pvss::INVALID_PROPERTY_TYPE;
            }
            if ( stat ) break;

        } else {
            prop.setStringValue( sValue.c_str() );
        }
        prop.setTimePolicy( policy, fd, lt );

        // modulus
        if ( cmdType() == PC_INC_MOD ) {
            uint32_t realmod = mod;
            if (ftModValue != ftUnknown) {
                REProperty *rp = context.getProperty(sMod);
                if (!rp) {
                    stat = pvss::PROPERTY_NOT_FOUND;
                    break;
                }
                realmod = static_cast<uint32_t>(rp->getInt());
            }
            command.setResult( realmod );
        }

    } while ( false );

    if ( stat ) setStatus( context, stat );
    // smsc_log_debug( logger, "property is: %s", prop.toString().c_str() );
    // prop.Serialize( sb );
    return stat;
}


void PersActionCommand::storeResults( const pvss::PersCommand& command, ActionContext& context )
{
    PersActionResultRetriever::storeResults( command, context );
    const pvss::PersCommandSingle* cmd = 
        const_cast< pvss::PersCommand& >(command).castSingle();
    assert(cmd);
    if ( 0 == command.status() ) {
        int result = 0;
        switch (cmdType()) {
        case (PC_DEL) :
        case (PC_SET) :
            break;
        case (PC_INC_RESULT) :
        case (PC_INC_MOD) : {
            // uint32_t i = sb.ReadInt32();
            REProperty* rp = context.getProperty(sResult);
            if (rp) rp->setInt( cmd->result() );
            break;
        }
        case (PC_GET) : {
            // prop.Deserialize( sb );
            REProperty* rp = context.getProperty( sValue );
            if (rp) {
                const pvss::Property& prop = cmd->property();
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
                    result = pvss::INVALID_PROPERTY_TYPE;
                }
            }
            break;
        }
        default :
            result = pvss::COMMAND_NOTSUPPORT;
        }
        if (result) setStatus( context, result );
    }
}


pvss::PersCommand* PersActionCommand::makeCommand( ActionContext& ctx )
{
    std::auto_ptr< pvss::PersCommand > res( new PersCommandSingle( cmdType() ) );
    if ( fillCommand(ctx,static_cast<PersCommandSingle&>(*res.get())) != 0 ) res.reset(0);
    return res.release();
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
        return scag::pvss::INFINIT;
    else if(!strcmp(str.c_str(), "FIXED"))
        return scag::pvss::FIXED;
    else if(!strcmp(str.c_str(), "ACCESS"))
        return scag::pvss::ACCESS;
    else if(!strcmp(str.c_str(), "R_ACCESS"))
        return scag::pvss::R_ACCESS;
    else if(!strcmp(str.c_str(), "W_ACCESS"))
        return scag::pvss::W_ACCESS;
    else
        return scag::pvss::UNKNOWN;
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
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", pvss::persCmdName(cmdType_));
    
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
                                    pvss::persCmdName(cmdType_), OPTIONAL_KEY, e.what());
            }
        } else if (strcmp(optionalKeyStr.c_str(), "0") && !(optionalKeyInt = atoi(optionalKeyStr.c_str()))) {
            throw SCAGException("PersAction '%s' : '%s' parameter not a number in not abonent profile type. key=%s",
                                pvss::persCmdName(cmdType_), OPTIONAL_KEY, optionalKeyStr.c_str());
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


std::auto_ptr< lcm::LongCallParams > PersActionBase::makeParams( ActionContext& context,
                                                                 PersActionResultRetriever& creator )
{
    std::auto_ptr< lcm::LongCallParams > res;
    do {

        if ( ! creator.canProcessRequest(context) ) break;

        PersCommand* cmd = creator.makeCommand(context);
        if ( ! cmd ) break;

        PersCallParams* params = new PersCallParams(profile,cmd);
        PersCall* data = params->getPersCall();
        res.reset( params );

        if ( !hasOptionalKey ) {
            CommandProperty& cp = context.getCommandProperty();
            switch (profile) {
            case (PT_ABONENT) : data->setKey( cp.abonentAddr.toString() ); break;
            case (PT_SERVICE) : data->setKey( cp.serviceId ); break;
            case (PT_OPERATOR) : data->setKey( cp.operatorId ); break;
            case (PT_PROVIDER) : data->setKey( cp.providerId ); break;
            default :
                creator.setStatus(context,pvss::BAD_REQUEST);
                res.reset(0);
            }
            break;
        }

        if ( ftOptionalKey != ftUnknown ) {
            REProperty *rp = context.getProperty(optionalKeyStr);
            if(!rp) {
                smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                               OPTIONAL_KEY, optionalKeyStr.c_str());
                creator.setStatus(context,pvss::PROPERTY_NOT_FOUND);
                res.reset(0);
                break;
            }
            if (profile == PT_ABONENT) {
                try {
                    data->setKey( getAbntAddress(rp->getStr().c_str()) );
                } catch(const std::runtime_error& e) {
                    smsc_log_error(logger, "'%s' parameter has error abonent profile key: %s",
                                   OPTIONAL_KEY, e.what());
                    creator.setStatus(context,pvss::INVALID_KEY);
                    res.reset(0);
                    break;
                }
            } else {
                data->setKey( static_cast<uint32_t>(rp->getInt()) );
            }
        } else if ( profile == PT_ABONENT ) {
            data->setKey( optionalKeyStr );
        } else {
            data->setKey( optionalKeyInt );
        }
    } while ( false );
    return res;
}


void PersActionBase::ContinueRunning(ActionContext& context)
{
    PersCallParams *params = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    PersCall* data = params->getPersCall();
    smsc_log_debug(logger, "ContinueRunning: cmd=%s (skey=%s ikey=%d)",
                   pvss::persCmdName(cmdType_),
                   data->getStringKey(),
                   data->getIntKey() );
    // params->readSB( context );
    // setStatus( context, params->status(), 0 );
    // persCommand.ContinueRunning(context);
    results().storeResults( * data->command(), context );
}


// =========================================================================


void PersAction::init(const SectionParams& params, PropertyObject propertyObject) 
{
    PersActionBase::init(params,propertyObject);
    persCommand.init(params,propertyObject);
}


bool PersAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s, profile=%d var=%s'...", pvss::persCmdName(cmdType_), profile, persCommand.propertyName());
    auto_ptr<lcm::LongCallParams> params = makeParams(context, persCommand);
    if ( ! params.get() ) return false;
    context.getSession().getLongCallContext().callCommandId = cmdToLongCallCmd(cmdType_);
    context.getSession().getLongCallContext().setParams(params.release());
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

}//actions
}//re
}//scag2
