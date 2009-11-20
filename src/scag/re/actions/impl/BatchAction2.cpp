#include "BatchAction2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/ActionFactory2.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/BatchResponse.h"
#include "util/PtrDestroy.h"

namespace scag2 { 

using namespace pvss;

namespace re {
namespace actions {

const std::string TRANSACTIONAL_MODE = "TRANSACTIONAL";
const std::string NORMAL_MODE = "NORMAL";
const char* BATCH_MODE = "mode";


BatchAction::~BatchAction()
{
    std::for_each( actions_.begin(), actions_.end(), smsc::util::PtrDestroy() );
}



void BatchAction::init(const SectionParams& params, PropertyObject propertyObject)
{
    smsc_log_debug(logger, "BatchAction: init");
    pobj_ = propertyObject;

    PersActionBase::init(params, propertyObject);

    bool statusExist = false;
    CheckParameter( params, propertyObject, "PersAction", "status", false, false,
                    batchStatus_, statusExist);
    bool msgExist = false;
    CheckParameter( params, propertyObject, "PersAction", "msg", false, false,
                    batchMsg_, msgExist);

    if (params.Exists(BATCH_MODE)) {
        if (TRANSACTIONAL_MODE == params[BATCH_MODE]) {
            transactMode_ = true;
        } else if (NORMAL_MODE == params[BATCH_MODE]) {
            transactMode_ = false;
        } else {
            transactMode_ = false;
            throw SCAGException("BatchAction : unknown 'mode' parameter %s", params[BATCH_MODE].c_str());
        }
    }

    if( !params.Exists("type") ||
        (scopeType_ = pvss::scopeTypeFromString(params["type"].c_str())) == pvss::ScopeType(0) )
        throw SCAGException("PersAction '%s' : missing or unknown 'type' parameter", typeToString());
}


bool BatchAction::RunBeforePostpone(ActionContext& context)
{
    if ( actions_.size() == 0 ) return false;
    smsc_log_debug(logger,"Run Action 'BatchAction' in %s mode...", transactMode_ ? TRANSACTIONAL_MODE.c_str() : NORMAL_MODE.c_str());
    PersCall* params = makeParams(context);
    if ( !params ) return false;
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;
    context.getSession().getLongCallContext().setParams( params );
    return true;
}


IParserHandler * BatchAction::StartXMLSubSection( const std::string& name, const SectionParams& params, const ActionFactory& factory )
{
    smsc_log_debug(logger, "BatchAction: %s", name.c_str());
    std::auto_ptr<PersActionCommand> act( (PersActionCommand*)( factory.CreateAction(name) ) );
    if ( !act.get() ) {
        throw SCAGException("batch action %s was not created", name.c_str());
    }
    act->init(params, pobj_);
    actions_.push_back(act.release());
    return NULL;
}


bool BatchAction::FinishXMLSubSection(const std::string& name)
{
    smsc_log_debug(logger, "BatchAction: FinishXMLSubsection %s", name.c_str());
    // NOTE: we have a buggy xerces_c parser at niagara, which returns batch:batch
    // instead of profile:batch so we'll check for both
    return (name=="profile:batch" || name == "batch:batch");
}


pvss::ProfileCommand* BatchAction::makeCommand( ActionContext& ctx )
{
    // batch.resize( actions.size() );
    std::vector< BatchRequestComponent* > comps;
    comps.reserve( actions_.size() );
    for ( std::vector< PersActionCommand* >::const_iterator i = actions_.begin();
          i != actions_.end();
          ++i ) {
        try {
            comps.push_back((*i)->makeCommand( ctx ));
        } catch ( PvssException& e ) {
            std::for_each( comps.begin(), comps.end(), smsc::util::PtrDestroy() );
            handleException(ctx,e,i);
        } catch ( std::exception& e ) {
            std::for_each( comps.begin(), comps.end(), smsc::util::PtrDestroy() );
            PvssException pe(e.what(),PvssException::UNKNOWN);
            handleException(ctx,pe,i);
        }
    }
    std::auto_ptr< BatchCommand > res(new BatchCommand);
    res->setTransactional(transactMode_);
    res->addComponents(comps);
    return res.release();
}


void BatchAction::handleResponse( ActionContext& ctx, const pvss::CommandResponse& resp )
{
    const BatchResponse& response = static_cast<const BatchResponse&>(resp);
    typedef std::vector< BatchResponseComponent* > BatchList;
    const BatchList& comps = response.getBatchContent();
    if ( comps.size() > actions_.size() ) {
        smsc_log_fatal(logger,"logic error in batch action: response size is bigger than orig size, abort will follow");
        ::abort();
    }
    std::vector< PersActionCommand* >::const_iterator j = actions_.begin();
    uint8_t status = PvssException::OK;
    std::string msg = PvssException::statusMessage(status);
    for ( BatchList::const_iterator i = comps.begin(); i != comps.end(); ++i ) {
        (*j)->handleResponse(ctx,**i);
        if ( (*i)->getStatus() != PvssException::OK && status == PvssException::OK ) {
            status = (*i)->getStatus();
            char buf[30];
            sprintf(buf," in action #%u",unsigned(i-comps.begin()));
            msg = std::string(PvssException::statusMessage(status)) + buf;
        }
    }
    setStatus(ctx,status,msg.c_str());
}


void BatchAction::handleError( ActionContext& ctx, const pvss::PvssException& e )
{
    setStatus( ctx, uint8_t(e.getType()), e.what() );
}


void BatchAction::handleException( ActionContext& ctx, const PvssException& e,
                                   std::vector< PersActionCommand* >::const_iterator i )
{
    (*i)->handleError(ctx,e);
    char buf[30];
    sprintf(buf," in action #%u", unsigned(i-actions_.begin()));
    std::string pewhat;
    pewhat.reserve(strlen(e.what())+30);
    pewhat.append(e.what());
    pewhat.append(buf);
    PvssException pe(pewhat,e.getType());
    throw pe;
}


/*
void BatchAction::storeResults( const pvss::PersCommand& command, ActionContext& context )
{
    const pvss::PersCommandBatch* cmd = 
        const_cast< pvss::PersCommand& >(command).castBatch();
    assert(cmd);
    const std::vector< pvss::PersCommandSingle >& batchcmds = cmd->batch();
    assert( batchcmds.size() == actions.size() );
    int stopat = actions.size();
    if ( command.status() ) stopat = command.failIndex();
    int idx = 0;
    for ( std::vector< PersActionCommand* >::iterator i = actions.begin();
          i != actions.end();
          ++i ) {
        if ( idx++ >= stopat ) break;
        (*i)->storeResults( batchcmds[idx], context );
    }
    PersActionResultRetriever::storeResults( command, context );
}
 */

}//actions
}//re
}//scag2
