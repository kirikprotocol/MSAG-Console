#include "PersAction2.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/lcm/base/LongCallManager2.h"
#include "scag/util/properties/Properties2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/ProfileResponse.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "scag/pvss/api/packets/IncResponse.h"
#include "sms/sms.h"
  
namespace {

using namespace scag2::pvss;
using namespace scag2::lcm;

const char* OPTIONAL_KEY = "key";

enum {
        CMDDEL = 1,
        CMDSET = 2,
        CMDGET = 3,
        CMDINC = 4,
        CMDINCMOD = 5,
        CMDBATCH = 6
};

static uint32_t cmdToLongCallCmd(uint32_t c)
{
    switch(c)
    {
    case CMDDEL:     return scag2::lcm::PERS_DEL;
    case CMDSET:     return scag2::lcm::PERS_SET;
    case CMDGET:     return scag2::lcm::PERS_GET;
    case CMDINC:     return scag2::lcm::PERS_INC;
    case CMDINCMOD:  return scag2::lcm::PERS_INC_MOD;
    default : break;
    }
    return 0;
}

struct CmdTypeGetter : public ProfileCommandVisitor, public ProfileResponseVisitor
{
    virtual bool visitDelCommand( DelCommand& ) { type = CMDDEL; return true; }
    virtual bool visitGetCommand( GetCommand& ) { type = CMDGET; return true; }
    virtual bool visitSetCommand( SetCommand& ) { type = CMDSET; return true; }
    virtual bool visitIncCommand( IncCommand& ) { type = CMDINC; return true; }
    virtual bool visitIncModCommand( IncModCommand& ) { type = CMDINCMOD; return true; }
    virtual bool visitBatchCommand( BatchCommand& ) { type = CMDBATCH; return true; }

    virtual bool visitDelResponse( DelResponse& ) { type = CMDDEL; return true; }
    virtual bool visitGetResponse( GetResponse& ) { type = CMDGET; return true; }
    virtual bool visitSetResponse( SetResponse& ) { type = CMDSET; return true; }
    virtual bool visitIncResponse( IncResponse& ) { type = CMDINC; return true; }
    virtual bool visitBatchResponse( BatchResponse& ) { type = CMDBATCH; return true; }

    unsigned type;
};

unsigned getCommandType( const ProfileCommand* cmd )
{
    if ( ! cmd ) return 0;
    CmdTypeGetter ctg;
    const_cast< ProfileCommand* >(cmd)->visit(ctg);
    return ctg.type;
}
unsigned getResponseType( const CommandResponse* resp )
{
    if ( ! resp ) return 0;
    CmdTypeGetter ctg;
    const_cast< CommandResponse* >(resp)->visit(ctg);
    return ctg.type;
}


/*
class PersCallParams : public LongCallParams
{
public:
    PersCallParams( ProfileRequest* req = 0 ) : request_(req), response_(0) {}
    virtual ~PersCallParams() {
        if (request_) delete request_; 
        if (response_) delete response_;
    }
    inline ProfileKey& getProfileKey() { return request_->getProfileKey(); }
    inline const ProfileKey& getProfileKey() const { return request_->getProfileKey(); }
    inline const CommandResponse* response() const {
        return response_ ? response_->getResponse() : 0;
    }
    
private:
    ProfileRequest* request_;
    ProfileResponse* response_;
};
 */

void setProfileIntKey( ProfileKey& pkey, ScopeType type, uint32_t key )
{
    switch (type) {
    case SCOPE_OPERATOR : pkey.setOperatorKey(key); break;
    case SCOPE_PROVIDER : pkey.setProviderKey(key); break;
    case SCOPE_SERVICE  : pkey.setServiceKey(key); break;
    default : throw PvssException( PvssException::BAD_REQUEST, "wrong scope (%d)", int(type) );
    }
}
}


namespace scag2 {
namespace re {
namespace actions { namespace reprop = util::properties;

typedef reprop::Property REProperty;
using namespace pvss;


bool PersActionResultRetriever::canProcessRequest( ActionContext& ctx )
{
    PvssException e;
    if ( !lcm::LongCallManager::Instance().pvssClient().canProcessRequest(&e) ) {
        // failed
        handleError(ctx,e);
        return false;
    }
    return true;
}


void PersActionResultRetriever::setStatus( ActionContext& context, uint8_t status, const char* msg )
{
    // __trace2__("pers: setting status %d, actionidx %d", status, actionIdx );
    REProperty *statusProp = context.getProperty(statusName());
    if (statusProp) {
        statusProp->setInt(status);
    }
    if (msg) {
        REProperty *msgProp = context.getProperty(msgName());
        if ( msgProp ) {
            msgProp->setStr(msg);
        }
    }
    /*
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
     */
}

/*
void PersActionResultRetriever::storeResults( const PersCommand& cmd, ActionContext& ctx )
{
    setStatus( ctx, cmd.status(), cmd.failIndex() );
}
*/


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

    const unsigned cmdType = getCommandType( cmdType_.get() );

    //////////////////////////////////////////////
    if (cmdType == CMDDEL)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", cmdType_->typeToString(), var.c_str());
        return;
    }

    if (cmdType == CMDINCMOD || cmdType == CMDINC) {
        sValue = params.Exists("inc") ? params["inc"] : "1";
        bool resultExist = false;
        CheckParameter(params, propertyObject, name(), "result", false, false, sResult, resultExist);
    }
    else
    {
        if(!params.Exists("value"))
            throw SCAGException("PersAction 'value' : missing '%s' parameter", cmdType_->typeToString());
        sValue = params["value"];
    }

    const char* nm = 0;
    ftValue = ActionContext::Separate(sValue, nm);
    if( cmdType == CMDGET && (ftValue == ftUnknown || ftValue == ftConst))
        throw InvalidPropertyException("PersAction '%s': 'value' parameter should be an lvalue. Got %s", cmdType_->typeToString(), sValue.c_str());

    if (ftValue == ftField) 
    {
        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, nm, propertyObject.transport);
        if (!(at & atRead) || (cmdType == CMDGET && !(at & atWrite)))
            throw InvalidPropertyException("PersAction '%s': cannot read/modify property '%s' - no access", sValue.c_str());
    }

    if (cmdType == CMDGET)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s", cmdType_->typeToString(), var.c_str());
        return;
    }

    if (cmdType == CMDINCMOD)
    {
        mod = 0;
        sMod = params.Exists("mod") ? params["mod"] : "0";

        ftModValue = ActionContext::Separate(sMod, nm); 
        if(ftModValue == ftField) 
        {
            AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, nm, propertyObject.transport);
            if(!(at & atRead)) 
                throw InvalidPropertyException("PersAction '%s': cannot read property '%s' - no access", cmdType_->typeToString(), sMod.c_str());
        }

        if(ftModValue == ftUnknown && strcmp(sMod.c_str(), "0") && !(mod = atoi(sMod.c_str())))
            throw SCAGException("PersAction '%s' : 'mod' parameter not a number. mod=%s", cmdType_->typeToString(), sMod.c_str());
    }

    if(!params.Exists("policy") || (policy = getPolicyFromStr(params["policy"])) == pvss::UNKNOWN )
        throw SCAGException("PersAction '%s' : missing or unknown 'policy' parameter", cmdType_->typeToString());

    if(policy == pvss::INFINIT)
    {
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, mod=%d", cmdType_->typeToString(), var.c_str(), policy, mod);
        return;
    }

    if(policy == pvss::FIXED && params.Exists("finaldate"))
    {

        ftFinalDate = CheckParameter(params, propertyObject, name(), "finaldate", true, true, sFinalDate, bExist);
        if(ftFinalDate == ftUnknown)
        {
            finalDate = parseFinalDate(sFinalDate.c_str());
            if(!finalDate)
                throw SCAGException("PersAction '%s' : invalid 'finaldate' parameter", cmdType_->typeToString());
        }
        
        smsc_log_debug(logger, "PersAction: params: cmd = %s, var=%s, policy=%d, final_date=%d", cmdType_->typeToString(), var.c_str(), policy, finalDate);
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
                   cmdType_->typeToString(), var.c_str(), policy, mod);
}

    
void PersActionCommand::handleResponse( ActionContext& context,
                                        const pvss::CommandResponse& resp )
{
    setStatus(context,resp.getStatus(),resp.statusToString(resp.getStatus()));
    if ( resp.getStatus() != PvssException::OK ) {
        return;
    }

    const unsigned cmdType = getResponseType( &resp );
    uint8_t result = PvssException::OK;
    switch (cmdType) {
    case (CMDDEL) :
    case (CMDSET) :
        break;
    case (CMDINC) : {
        // uint32_t i = sb.ReadInt32();
        REProperty* rp = context.getProperty(sResult);
        if (rp) rp->setInt( static_cast<const IncResponse&>(resp).getResult() );
        break;
    }
    case (CMDGET) : {
        // prop.Deserialize( sb );
        REProperty* rp = context.getProperty( sValue );
        if (rp) {
            const pvss::Property& prop = 
                static_cast<const GetResponse&>(resp).getProperty();
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
                result = PvssException::TYPE_INCONSISTENCE;
            }
        }
        break;
    }
    default :
        result = PvssException::NOT_SUPPORTED;
    }
    if (result != PvssException::OK) {
        setStatus(context,result,PvssException::statusMessage(result));
    }
}


void PersActionCommand::handleError( ActionContext& ctx,
                                     const pvss::PvssException& exc )
{
    setStatus( ctx, uint8_t(exc.getType()),
               exc.getMessage().empty() ?
               exc.statusToString(exc.getType()) :
               exc.getMessage().c_str() );
}


BatchRequestComponent* PersActionCommand::makeCommand( ActionContext& context )
{
    // --- get property name
    std::auto_ptr<BatchRequestComponent> retval;
    do {

        const unsigned cmdType = getCommandType( cmdType_.get() );

        REProperty *p;
        if (ftVar != ftUnknown && !(p = context.getProperty(var.c_str()))) {
            throw PvssException( PvssException::PROPERTY_NOT_FOUND, "Invalid var property %s", var.c_str());
        }

        const REProperty::string_type& svar = ( ftVar == ftUnknown ? var : p->getStr() );
        if ( cmdType == CMDDEL ) {
            DelCommand* ret = static_cast<DelCommand*>(cmdType_->clone());
            retval.reset(ret);
            ret->setVarName(svar.c_str());
            break;
        } else if ( cmdType == CMDGET ) {
            GetCommand* ret = static_cast<GetCommand*>(cmdType_->clone());
            retval.reset(ret);
            ret->setVarName(svar.c_str());
            break;
        }

        if ( cmdType != CMDSET && cmdType != CMDINC && cmdType != CMDINCMOD ) {
            throw PvssException( PvssException::NOT_SUPPORTED, "Invalid command %s", cmdType_->typeToString() );
        }

        AbstractPropertyCommand* ret = static_cast<AbstractPropertyCommand*>(cmdType_->clone());
        retval.reset(ret);
        pvss::Property& prop = ret->getProperty();
        prop.setName( svar.c_str() );

        // --- time policy
        time_t fd = finalDate;
        uint32_t lt = lifetime_.getTime( name(), context );
        if (ftFinalDate != ftUnknown)  {
            REProperty *rp = context.getProperty(sFinalDate);
            if (!rp || !(fd = parseFinalDate(rp->getStr().c_str()))) {
                throw PvssException( PvssException::PROPERTY_NOT_FOUND, "Invalid finaldate parameter %s(%s)", sFinalDate.c_str(), rp ? rp->getStr().c_str() : "");
            }
        }
        prop.setTimePolicy( policy, fd, lt );

        // modulus
        if ( cmdType == CMDINCMOD ) {
            uint32_t realmod = mod;
            if (ftModValue != ftUnknown) {
                REProperty *rp = context.getProperty(sMod);
                if (!rp) {
                    throw PvssException(PvssException::PROPERTY_NOT_FOUND,"modulus property %s not found",sMod.c_str());
                }
                realmod = static_cast<uint32_t>(rp->getInt());
            }
            static_cast<IncModCommand*>(ret)->setModulus( realmod );
        }

        if (ftValue != ftUnknown)
        {
            REProperty *rp = context.getProperty(sValue);
            if (!rp) {
                throw PvssException( PvssException::PROPERTY_NOT_FOUND, "Value property %s not found", sValue.c_str() );
            }

            if ( cmdType == CMDINC || cmdType == CMDINCMOD ) {
                prop.setIntValue(rp->getInt());
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
                throw PvssException( PvssException::TYPE_INCONSISTENCE, "Wrong property type %d", int(rp->getType()) );
            }

        } else {
            if ( cmdType == CMDINC || cmdType == CMDINCMOD ) {
                char* endptr;
                prop.setIntValue( strtoull(sValue.c_str(),&endptr,0) );
                if ( *endptr != '\0' ) {
                    throw PvssException( PvssException::TYPE_INCONSISTENCE, "Wrong property content for inc/incmod");
                }
                break;
            }
            prop.setStringValue( sValue.c_str() );
        }

    } while ( false );
    return retval.release();
}


/*
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
 */


/*
pvss::PersCommand* PersActionCommand::makeCommand( ActionContext& ctx )
{
    std::auto_ptr< pvss::PersCommand > res( new PersCommandSingle( cmdType() ) );
    if ( fillCommand(ctx,static_cast<PersCommandSingle&>(*res.get())) != 0 ) res.reset(0);
    return res.release();
}
 */


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
        return pvss::INFINIT;
    else if(!strcmp(str.c_str(), "FIXED"))
        return pvss::FIXED;
    else if(!strcmp(str.c_str(), "ACCESS"))
        return pvss::ACCESS;
    else if(!strcmp(str.c_str(), "R_ACCESS"))
        return pvss::R_ACCESS;
    else if(!strcmp(str.c_str(), "W_ACCESS"))
        return pvss::W_ACCESS;
    else
        return pvss::UNKNOWN;
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
    if ( !params.Exists("type") || 
         (scopeType_ = pvss::scopeTypeFromString(params["type"].c_str())) == pvss::ScopeType(0) ) {
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", typeToString());
    }
    
    ftOptionalKey = CheckParameter( params,propertyObject, "persaction", OPTIONAL_KEY, false, true,
                                    optionalKeyStr, hasOptionalKey );
    if (hasOptionalKey && ftOptionalKey == ftUnknown) {
        smsc_log_debug(logger, "PersAction '%s': optional_key_str=%s, optional key exists",
                       typeToString(), optionalKeyStr.c_str(), hasOptionalKey);

        if (scopeType_ == pvss::SCOPE_ABONENT) {
            try {
                optionalKeyStr = getAbntAddress(optionalKeyStr.c_str());
            } catch(const std::runtime_error& e) {
                throw SCAGException("PersAction '%s' : '%s' parameter has error abonent profile key: %s",
                                    typeToString(), OPTIONAL_KEY, e.what());
            }
        } else if (strcmp(optionalKeyStr.c_str(), "0") && !(optionalKeyInt = atoi(optionalKeyStr.c_str()))) {
            throw SCAGException("PersAction '%s' : '%s' parameter not a number in not abonent profile type. key=%s",
                                typeToString(), OPTIONAL_KEY, optionalKeyStr.c_str());
        }
    }
}


/*
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
 */


std::string PersActionBase::getAbntAddress(const char* _address) {
    Address address(_address);
    if (_address[0] != '.') {
        address.setNumberingPlan(1);
        address.setTypeOfNumber(1);
    }
    return address.toString();
}


PersCall* PersActionBase::makeParams( ActionContext& context )
{
    std::auto_ptr< PersCall > res;
    try {

        // NOTE: result handler will automatically handle error in case of false
        if ( ! resultHandler().canProcessRequest(context) ) return 0;

        ProfileCommand* cmd = resultHandler().makeCommand(context);
        assert( cmd );

        ProfileRequest* params = new ProfileRequest(cmd);
        res.reset(new PersCall(params));

        if ( !hasOptionalKey ) {
            CommandProperty& cp = context.getCommandProperty();
            switch (scopeType_) {
            case (SCOPE_ABONENT) : params->getProfileKey().setAbonentKey( cp.abonentAddr.toString() ); break;
            case (SCOPE_SERVICE) : params->getProfileKey().setServiceKey( cp.serviceId ); break;
            case (SCOPE_OPERATOR) : params->getProfileKey().setOperatorKey( cp.operatorId ); break;
            case (SCOPE_PROVIDER) : params->getProfileKey().setProviderKey( cp.providerId ); break;
            default :
                throw PvssException(PvssException::BAD_REQUEST,"wrong scope (%d)", int(scopeType_));
            }

        } else if ( ftOptionalKey != ftUnknown ) {

            REProperty *rp = context.getProperty(optionalKeyStr);
            if(!rp) {
                throw PvssException( PvssException::PROPERTY_NOT_FOUND,
                                     "'%s' parameter '%s' not found in action context",
                                     OPTIONAL_KEY, optionalKeyStr.c_str());
            }

            if (scopeType_ == SCOPE_ABONENT) {
                try {
                    params->getProfileKey().setAbonentKey( getAbntAddress(rp->getStr().c_str()) );
                } catch(const std::runtime_error& e) {
                    throw PvssException(PvssException::INVALID_KEY,
                                        "'%s' parameter has wrong abonent profile key: %s",
                                        OPTIONAL_KEY, e.what());
                }
            } else {
                setProfileIntKey( params->getProfileKey(), scopeType_, uint32_t(rp->getInt()) );
            }
        } else if ( scopeType_ == SCOPE_ABONENT ) {
            params->getProfileKey().setAbonentKey( optionalKeyStr );
        } else {
            setProfileIntKey( params->getProfileKey(), scopeType_, optionalKeyInt );
        }
    } catch ( PvssException& e ) {
        resultHandler().handleError( context, e );
        res.reset(0);
    } catch ( std::exception& e ) {
        resultHandler().handleError( context, PvssException(PvssException::UNKNOWN, e.what()) );
        res.reset(0);
    }
    return res.release();
}


void PersActionBase::ContinueRunning(ActionContext& context)
{
    PersCall *params = (PersCall*)context.getSession().getLongCallContext().getParams();
    // PersCall* data = params->getPersCall();
    ProfileKey* pkey = params->getProfileKey();
    smsc_log_debug(logger, "ContinueRunning: cmd=%s (%s)",
                   typeToString(), pkey ? pkey->toString().c_str() : "???" );
    // params->readSB( context );
    // setStatus( context, params->status(), 0 );
    // persCommand.ContinueRunning(context);
    if ( params->getResponse() && params->getResponse()->getResponse() ) {
        resultHandler().handleResponse( context, *params->getResponse()->getResponse() );
    } else {
        resultHandler().handleError( context, params->getException() );
    }
    // results().storeResults( * data->command(), context );
}


// =========================================================================


void PersAction::init(const SectionParams& params, PropertyObject propertyObject) 
{
    PersActionBase::init(params,propertyObject);
    persCommand.init(params,propertyObject);
}


bool PersAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'PersAction cmd=%s, scope=%s var=%s'...",
                   typeToString(), scopeTypeToString(scopeType_), persCommand.propertyName());
    PersCall* params = makeParams(context);
    if ( ! params ) return false;
    context.getSession().getLongCallContext().callCommandId =
        cmdToLongCallCmd( getCommandType( params->getRequest()->getCommand() ) );
    context.getSession().getLongCallContext().setParams(params);
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
