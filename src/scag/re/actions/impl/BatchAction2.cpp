#include "BatchAction2.h"
#include "scag/stat/Statistics.h"
// #include "scag/util/properties/Properties2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/ActionFactory2.h"
#include "scag/pers/util/PersClient2.h"

namespace scag2 { 
namespace re {
namespace actions {

// using util::properties::Property;
using namespace pers::util;
using namespace scag::stat;

const std::string TRANSACTIONAL_MODE = "TRANSACTIONAL";
const std::string NORMAL_MODE = "NORMAL";
const char* BATCH_MODE = "mode";


BatchAction::~BatchAction()
{
    for ( std::vector< PersActionCommand* >::iterator i = actions.begin();
          i != actions.end();
          ++i ) {
        if (*i) delete *i;
    }
}



void BatchAction::init(const SectionParams& params, PropertyObject propertyObject)
{
    smsc_log_debug(logger, "BatchAction: init");
    pobj = propertyObject;

    PersActionBase::init(params, propertyObject);

    bool statusExist = false;
    CheckParameter(params, propertyObject, "PersAction", "status", false, false,
                    batchStatus, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, "PersAction", "msg", false, false,
                    batchMsg, msgExist);

    if (params.Exists(BATCH_MODE)) {
        if (TRANSACTIONAL_MODE == params[BATCH_MODE]) {
            transactMode = true;
        } else if (NORMAL_MODE == params[BATCH_MODE]) {
            transactMode = false;
        } else {
            transactMode = false;
            throw SCAGException("BatchAction : unknown 'mode' parameter %s", params[BATCH_MODE].c_str());
        }
    }

    if(!params.Exists("type") || (profile = getProfileTypeFromStr(params["type"])) == PT_UNKNOWN) 
        throw SCAGException("PersAction 'batch' : missing or unknown 'type' parameter");
}


bool BatchAction::RunBeforePostpone(ActionContext& context)
{
    if ( actions.size() == 0 ) return false;
    smsc_log_debug(logger,"Run Action 'BatchAction' in %s mode...", transactMode ? TRANSACTIONAL_MODE.c_str() : NORMAL_MODE.c_str());
    auto_ptr< PersCallParams > params = makeParams(context,*this);
    if ( ! params.get() ) return false;
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;
    context.getSession().getLongCallContext().setParams( params.release() );
    return true;
}


/*
void BatchAction::ContinueRunning(ActionContext& context)
{
    smsc_log_debug(logger,"ContinueRunning Action 'BatchAction'...");
    PersCallParams* p = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    if (p->error) {
        smsc_log_debug(logger, "'BatchAction' abort. Error code=%d : %s", p->error, p->exception.c_str());
        p->setStatus( context, p->error );
        return;
    }
    smsc_log_debug(logger,"ContinueRunning Action 'BatchAction' (skey=%s ikey=%d)",
                   p->getStringKey(), p->getIntKey() );
    try {
        p->readSB( context );
        // setStatus(context, result, batchStatus, batchMsg, error_result_idx);
    } catch (const PersClientException& e) {
        p->setStatus(context, e.getType());
        smsc_log_debug(logger, "'BatchAction' abort. Error code=%d", e.getType(), e.what());
    }
}
 */

IParserHandler * BatchAction::StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory)
{
    smsc_log_debug(logger, "BatchAction: %s", name.c_str());
    std::auto_ptr<PersActionCommand> act( (PersActionCommand*)( factory.CreateAction(name) ) );
    act->init(params, pobj);
    actions.push_back(act.release());
    return NULL;
}


bool BatchAction::FinishXMLSubSection(const std::string& name)
{
    smsc_log_debug(logger, "BatchAction: FinishXMLSubsection %s", name.c_str());
    //return (name=="profile:batch");
    return (name=="batch:batch");
}


std::auto_ptr< pers::util::PersCommand > BatchAction::makeCommand( ActionContext& ctx )
{
    std::auto_ptr< pers::util::PersCommand > res;
    std::vector< pers::util::PersCommandSingle > batch;
    batch.resize( actions.size() );
    for ( std::vector< PersActionCommand* >::const_iterator i = actions.begin();
          i != actions.end();
          ++i ) {
        int stat = (*i)->fillCommand( ctx, batch[i-actions.begin()] );
        if ( stat ) {
            setStatus( ctx, stat, i-actions.begin() + 1);
            return res;
        }
    }
    res.reset( new pers::util::PersCommandBatch(*this,batch,transactMode) );
    return res;
}


}//actions
}//re
}//scag2
