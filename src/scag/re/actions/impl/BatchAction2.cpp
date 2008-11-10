#include "BatchAction2.h"
#include "scag/stat/Statistics.h"
// #include "scag/util/properties/Properties2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/ActionFactory2.h"
#include "scag/pers/util/PersClient2.h"

namespace {

class ConnectionCheck : public scag2::pers::util::PropertyProxyBase
{
public:
    ConnectionCheck( const std::string& stat, const std::string& msg ) :
    stat_(stat), msg_(msg) {}
    virtual const std::string& statusName() const { return stat_; }
    virtual const std::string& msgName() const { return msg_; }
private:
    const std::string& stat_;
    const std::string& msg_;
};

}


namespace scag2 { 
namespace re {
namespace actions {

// using util::properties::Property;
using namespace pers::util;
using namespace scag::stat;

const std::string TRANSACTIONAL_MODE = "TRANSACTIONAL";
const std::string NORMAL_MODE = "NORMAL";
const char* BATCH_MODE = "mode";


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
    if ( proxies.size() == 0 ) return false;

    smsc_log_debug(logger,"Run Action 'BatchAction' in %s mode...", transactMode ? TRANSACTIONAL_MODE.c_str() : NORMAL_MODE.c_str());
    ::ConnectionCheck ck( batchStatus, batchMsg );
    if ( ! ck.canProcessRequest(context) ) {
        return false;
    }
    auto_ptr<PersCallParams> params(new PersCallParams(profile, batchStatus, batchMsg));

    if ( !setKey(context, params.get()) ) {
        return false;
    }

    // prepare batch
    params->batch( proxies, transactMode );
    if ( !params->fillSB(context) ) {
        return false;
    }
    // failed to fill serial buffer (property not found?)
    // params->error = scag::pers::util::BATCH_ERROR;
    // params->exception = scag::pers::util::strs[scag::pers::util::BATCH_ERROR];
    // setStatus(context, params->error, batchStatus, batchMsg, i + 1);
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;
    context.getSession().getLongCallContext().setParams( params.release() );
    return true;
}


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
                   p->proxy().getKey().skey, p->proxy().getKey().ikey );
    try {
        p->readSB( context );
        /*
        int result = 0;
        int error_result_idx = 0;
        for ( int i = 0; i < actions.size(); i++) {
            action_idx = i + 1;
            int action_result = actions[i]->batchResult( context, p->sb, transactMode );
            setStatus(context, action_result, actions[i]->getStatus(), actions[i]->getMsg());
            if (!result) {
                result = action_result;
                error_result_idx = i + 1;
            }
        }
         */
        // setStatus(context, result, batchStatus, batchMsg, error_result_idx);
    } catch (const PersClientException& e) {
        p->setStatus(context, e.getType());
        smsc_log_debug(logger, "'BatchAction' abort. Error code=%d", e.getType(), e.what());
    }
}

IParserHandler * BatchAction::StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory)
{
    smsc_log_debug(logger, "BatchAction: %s", name.c_str());
    PersActionCommand* act = (PersActionCommand*)( factory.CreateAction(name) );
    act->init(params, pobj);
    actions.push_back(act);
    proxies.push_back(act);
    return NULL;
}

bool BatchAction::FinishXMLSubSection(const std::string& name)
{
    smsc_log_debug(logger, "BatchAction: FinishXMLSubsection %s", name.c_str());
    //return (name=="profile:batch");
    return (name=="batch:batch");
}

}//actions
}//re
}//scag2
