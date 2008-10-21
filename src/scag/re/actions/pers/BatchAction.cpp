#include "BatchAction.h"
#include "scag/stat/Statistics.h"
#include "scag/util/properties/Properties.h"
#include "scag/re/CommandAdapter.h"
#include "scag/re/actions/ActionFactory.h"

namespace scag { namespace pers {

typedef scag::util::properties::Property REProperty;

using namespace scag::stat;
using namespace scag::pers::client;

const std::string TRANSACTIONAL_MODE = "TRANSACTIONAL";
const std::string NORMAL_MODE = "NORMAL";
const char* BATCH_MODE = "mode";
const size_t ACTION_IDX_BUFFER_SIZE = 10;

void BatchAction::init(const SectionParams& params, PropertyObject propertyObject)
{
	smsc_log_debug(logger, "BatchAction: init");
	pobj = propertyObject;

    PersActionBase::init(params, propertyObject);

    bool statusExist = false;
    CheckParameter(params, propertyObject, "PersAction", "status", false, false,
                    status_str, statusExist);
    bool msgExist = false;
    CheckParameter(params, propertyObject, "PersAction", "msg", false, false,
                    msg_str, msgExist);

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
    smsc_log_debug(logger,"Run Action 'BatchAction' in %s mode..."
                   , transactMode ? TRANSACTIONAL_MODE.c_str() : NORMAL_MODE.c_str());
	auto_ptr<PersCallParams> p(new PersCallParams());
    context.getSession().getLongCallContext().callCommandId = PERS_BATCH;
    CommandProperty& cp = context.getCommandProperty();
    PersKey pk;
    std::string skey;

    if (hasOptionalKey) {
      if (ftOptionalKey == ftUnknown) {
        if (profile == PT_ABONENT) {
          pk.skey = optionalSkey.c_str();
          skey = optionalSkey;
        } else {
          pk.ikey = optionalIkey;
        }
      } else {
        REProperty *rp = context.getProperty(optionalKeyStr);
        if(!rp) {
          smsc_log_error(logger, "'%s' parameter '%s' not found in action context",
                          OPTIONAL_KEY, optionalKeyStr.c_str());
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
          pk.ikey = static_cast<uint32_t>(rp->getInt());
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

    PersClient::Instance().PrepareMTBatch(p->sb, profile, pk, static_cast<uint16_t>(actions.size()), transactMode);
    for(int i = 0; i < actions.size(); i++) {
      if (!actions[i]->batchPrepare(profile, context, p->sb)) {
        p->error = BATCH_ERROR;
        p->exception = strs[BATCH_ERROR];
        setStatus(context, p.get(), p->error, p->exception, i + 1);
      }
    }
    context.getSession().getLongCallContext().setParams(p.release());
    return true;
}

void BatchAction::ContinueRunning(ActionContext& context)
{
    smsc_log_debug(logger,"ContinueRunning Action 'BatchAction'...");
	PersCallParams* p = (PersCallParams*)context.getSession().getLongCallContext().getParams();
    if (p->error) {
      smsc_log_debug(logger, "'BatchAction' abort. Error code=%d : %s", p->error, p->exception.c_str());
      return;
    }
    smsc_log_debug(logger,"ContinueRunning Action 'BatchAction' (skey=%s ikey=%d)", p->skey.c_str(), p->ikey);
    int action_idx = 0;
    try {
      int result = 0;
      int error_result_idx = 0;
      for(int i = 0; i < actions.size(); i++) {
        action_idx = i + 1;
        int action_result = actions[i]->batchResult(context, p->sb, transactMode);
        if (!result) {
          result = action_result;
          error_result_idx = i + 1;
        }
      }
      setStatus(context, p, result, result == 0 ? "Ok" : strs[result], error_result_idx);
    } catch (const PersClientException& e) {
      setStatus(context, p, e.getType(), e.what(), action_idx);
      smsc_log_debug(logger, "'BatchAction' abort. Error code=%d : %s in action %d", e.getType(), e.what(), action_idx);
    }
}

void BatchAction::setStatus(ActionContext& context, PersCallParams *params, int status, const std::string& msg, int action_idx) {
  std::string _msg = msg;
  if (status != 0) {
    char idx_buffer[ACTION_IDX_BUFFER_SIZE];
    int n = snprintf(idx_buffer, ACTION_IDX_BUFFER_SIZE, "%d", action_idx);
    _msg += " in action ";
    _msg.append(idx_buffer, n);
  }
  REProperty *statusProp = context.getProperty(status_str);
  if (statusProp) {
    statusProp->setInt(status);
  }
  REProperty *msgProp = context.getProperty(msg_str);
  if (msgProp) {
    msgProp->setStr(_msg);
  }
}


IParserHandler * BatchAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
	smsc_log_debug(logger, "BatchAction: %s", name.c_str());
	PersActionCommand* act = (PersActionCommand*)factory.CreateAction(name);
	act->init(params, pobj);
	actions.push_back(act);
	return NULL;
}

bool BatchAction::FinishXMLSubSection(const std::string& name)
{
	smsc_log_debug(logger, "BatchAction: FinishXMLSubsection %s", name.c_str());
    //return (name=="profile:batch");
    return (name=="batch:batch");
}

}}
