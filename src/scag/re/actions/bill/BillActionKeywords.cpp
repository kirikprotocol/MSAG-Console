#include "BillActionKeywords.h"
#include "scag/re/CommandAdapter.h"

namespace scag {
namespace re {
namespace actions {

const char* KEYWORDS_NAME = "keywords";

void BillActionKeywords::init(const SectionParams &params, PropertyObject propertyObject) {
  m_BillId = 0;
  FieldType ft = CheckParameter(params, propertyObject, opname(), "bill_id", false, true, m_sBillId, bExist);
  CheckParameter(params, propertyObject, opname(), "status", false, false, m_sStatus, m_StatusExist);
  CheckParameter(params, propertyObject, opname(), "msg", false, false, m_sMessage, m_MsgExist);
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, opname(), KEYWORDS_NAME, true, true, keywords_, keywordsExists);
  smsc_log_debug(logger,"Action '%s' init...", opname());
}

bool BillActionKeywords::run(ActionContext& context)
{
    Operation *op = NULL;
    if(bExist)
    {
        if(!m_BillId)
        {
            Property * p = context.getProperty(m_sBillId);
            if(!p || !(m_BillId = p->getInt()))
            {
                std::string s = "Action '" + string(opname()) + " ' :: Invalid property " + m_sBillId + " for BillID";
                smsc_log_error(logger, s.c_str());
                SetBillingStatus(context, s.c_str(), false);
                return false;
            }
        }
    }
    else
    {
        op = context.GetCurrentOperation();
        if (!op || !op->hasBill())
        {
            const char *p = !op ? "Bill: Operation from ActionContext is invalid" : "Bill is not attached to operation";
            smsc_log_error(logger, p);
            SetBillingStatus(context, p, false);
            return false;
        }
        m_BillId = op->getBillId();
    }
    return true;
}

void BillActionKeywords::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK)
{
    Property * property;
    if(m_StatusExist) 
    {
        if(!(property = context.getProperty(m_sStatus))) 
        {
            smsc_log_debug(logger,"BillAction '%s' :: Invalid property %s for status", opname(), m_sStatus.c_str());
            return;
        }
        property->setInt(!isOK);
    }

    if(m_MsgExist) 
    {
        if(!(property = context.getProperty(m_sMessage))) 
        {
            smsc_log_debug(logger,"BillAction '%s' :: Invalid property %s for msg", opname(), m_sMessage.c_str());
            return;
        }
        property->setStr(std::string(errorMsg));
    }
    return;
}

bool BillActionKeywords::FinishXMLSubSection(const std::string &name) {
  return true;
}

IParserHandler * BillActionKeywords::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) {
  throw SCAGException("Action '%s' cannot include child objects", opname());
}

bool BillActionSetKeywords::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'...", opname());
  if (!BillActionKeywords::run(context)) {
    return true;
  }
  SetBillingStatus(context, "", true);
  return true;
}

bool BillActionAddKeywords::run(ActionContext &context) {
  if (!BillActionKeywords::run(context)) {
    return true;
  }
  SetBillingStatus(context, "", true);
  return true;
}

}
}
}
