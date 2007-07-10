#include "BatchAction.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/CommandAdapter.h"
#include "scag/pers/PersClient.h"
#include "scag/pers/Property.h"
#include "scag/re/actions/ActionFactory.h"

namespace scag { namespace pers {

using namespace scag::stat;
using namespace scag::pers::client;

void BatchAction::init(const SectionParams& params, PropertyObject propertyObject)
{
	smsc_log_debug(logger, "BatchAction: init");
	pobj = propertyObject;
}

bool BatchAction::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'BatchAction'...");
	auto_ptr<PersCallParams> p(new PersCallParams());
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;	
	PersClient::Instance().PrepareBatch(p->sb);
    for(int i = 0; i < actions.size(); i++)
        actions[i]->batchPrepare(context, p->sb);
	PersClient::Instance().FinishPrepareBatch(actions.size(), p->sb);
    context.getSession().getLongCallContext().setParams(p.release());
    return true;
}

void BatchAction::ContinueRunning(ActionContext& context)
{
    smsc_log_debug(logger,"ContinueRunning Action 'BatchAction'...");
	PersCallParams* p = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    for(int i = 0; i < actions.size(); i++)
        actions[i]->batchResult(context, p->sb);
}

IParserHandler * BatchAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
	smsc_log_debug(logger, "BatchAction: %s", name.c_str());
	PersBatchAction* act = (PersBatchAction*)factory.CreateAction(name);
	act->init(params, pobj);
	actions.push_back(act);
	return NULL;
}

bool BatchAction::FinishXMLSubSection(const std::string& name)
{
	smsc_log_debug(logger, "BatchAction: FinishXMLSubsection %s", name.c_str());
    return (name=="profile:batch");
}

}}
