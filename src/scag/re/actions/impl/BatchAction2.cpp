#include "BatchAction2.h"
#include "scag/stat/Statistics.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/ActionFactory2.h"
#include "scag/pers/util/PersClient2.h"

namespace scag2 { 
namespace re {
namespace actions {

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
    auto_ptr< lcm::LongCallParams > params = makeParams(context,*this);
    if ( ! params.get() ) return false;
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;
    context.getSession().getLongCallContext().setParams( params.release() );
    return true;
}


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
    // NOTE: we have a buggy xerces_c parser at niagara, which returns batch:batch
    // instead of profile:batch so we'll check for both
    return (name=="profile:batch" || name == "batch:batch");
}


pers::util::PersCommand* BatchAction::makeCommand( ActionContext& ctx )
{
    std::auto_ptr< pers::util::PersCommand > res;
    std::vector< pers::util::PersCommandSingle > batch;
    batch.resize( actions.size() );
    for ( std::vector< PersActionCommand* >::const_iterator i = actions.begin();
          i != actions.end();
          ++i ) {
        int stat = (*i)->fillCommand( ctx, batch[i - actions.begin()] );
        if ( stat ) {
            setStatus( ctx, stat, i-actions.begin() + 1);
            return res.release();
        }
    }
    res.reset( new pers::util::PersCommandBatch(batch,transactMode) );
    return res.release();
}


}//actions
}//re
}//scag2
