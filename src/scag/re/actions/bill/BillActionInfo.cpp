#include "BillActionInfo.h"
#include "scag/re/CommandAdapter.h"
#include "scag/util/lltostr.h"

namespace scag { namespace re { namespace actions {

const char* BillActionInfo::m_name[BillActionInfo::fields_count] = {"operator_id", "provider_id", "service_id", "abonent", "category", "content-type", "result_number", "price", "currency", "bill_type", "category-str", "content-type-str", "keywords"};

void BillActionInfo::init(const SectionParams& params,PropertyObject propertyObject)
{
    m_BillId = 0;
    FieldType ft = CheckParameter(params, propertyObject, "bill:info", "bill_id", false, true, m_sBillId, bExist);
    if(bExist && ft == ftUnknown && !(m_BillId = atoi(m_sBillId.c_str())))
        throw InvalidPropertyException("Action 'bill:info': bill_id should be positive integer value");

    for(int i = 0; i < fields_count; i++)
        CheckParameter(params, propertyObject, "bill:info", m_name[i], false, false, m_sField[i], m_exist[i]);

    CheckParameter(params, propertyObject, "bill:info", "status", false, false, m_sStatus, m_StatusExist);
    CheckParameter(params, propertyObject, "bill:info", "msg", false, false, m_sMessage, m_MsgExist);

    smsc_log_debug(logger,"Action 'bill:info' init...");
}


IParserHandler * BillActionInfo::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'bill:info' cannot include child objects");
}

bool BillActionInfo::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void BillActionInfo::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK)
{
    Property * property;
    if(m_StatusExist) 
    {
        if(!(property = context.getProperty(m_sStatus))) 
        {
            smsc_log_debug(logger,"BillAction 'bill:info' :: Invalid property %s for status", m_sStatus.c_str());
            return;
        }
        property->setInt(!isOK);
    }

    if(m_MsgExist) 
    {
        if(!(property = context.getProperty(m_sMessage))) 
        {
            smsc_log_debug(logger,"BillAction 'bill:info' :: Invalid property %s for msg", m_sMessage.c_str());
            return;
        }
        property->setStr(std::string(errorMsg));
    }
    return;
}

bool BillActionInfo::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'bill:info'...");
    Operation *op = NULL;
    uint32_t bid = m_BillId;

    if(bExist)
    {
        if(!bid)
        {
            Property * p = context.getProperty(m_sBillId);
            if(!p || !(bid = p->getInt()))
            {
                std::string s = "Action 'bill:info' :: Invalid property " + m_sBillId + " for BillID";
                smsc_log_error(logger, s.c_str());
                SetBillingStatus(context, s.c_str(), false);
                return true;
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
            return true;
        }
        bid = op->getBillId();
    }

    BillingInfoStruct bis;
    TariffRec tr;

    try{
        BillingManager::Instance().Info(bid, bis, tr);
    }
    catch(SCAGException& e)
    {
        smsc_log_error(logger,"Action 'bill:info' :: No transaction with bill_id=%d. Error: %s", bid, e.what());
        SetBillingStatus(context, e.what(), false);
        return true;
    }

    char buf[20];
    buf[19] = 0;
    std::string s = "Action 'bill:info' bill_id=";
    s += lltostr(bid, buf + 19);
    for(int i = 0; i < fields_count; i++) 
    {
        if(!m_exist[i]) continue;
        Property *p = context.getProperty(m_sField[i]);
        if(!p)
        {
            std::string s = "Action 'bill:info' :: Invalid property " + m_sField[i] + " for " +  m_name[i];
            smsc_log_error(logger, s.c_str());
            SetBillingStatus(context, s.c_str(), false);
            return true;
        }
        switch(i)
        {
            case 0: p->setInt(bis.operatorId); break;
            case 1: p->setInt(bis.providerId); break;
            case 2: p->setInt(bis.serviceId); break;
            case 3: p->setStr(bis.AbonentNumber); break;
            case 4: p->setInt(tr.CategoryId); break;
            case 5: p->setInt(tr.MediaTypeId); break;
            case 6: p->setStr(tr.ServiceNumber); break;
            case 7:
                {
                    char buf[20];
                    sprintf(buf, "%.3lf", tr.Price);
                    p->setStr(buf);
                    break;
                }
            case 8: p->setStr(tr.Currency); break;
            case 9: p->setInt(tr.billType); break;
            case 10: p->setStr(bis.category); break;
            case 11: p->setStr(bis.mediaType); break;
            case 12:
            {
              const std::string* keywords = op->getKeywords();
              if (keywords) {
                p->setStr(*keywords); 
              } else {
                p->setStr("");
              }
              break;
            }
                  
        }
        s += ", ";
        s += m_name[i];
        s += "=" + p->getStr();
    }
    smsc_log_debug(logger, s);
    SetBillingStatus(context, "", true);
    return true;
}

}}}

