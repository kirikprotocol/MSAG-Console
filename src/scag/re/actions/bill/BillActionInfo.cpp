#include "BillActionInfo.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

const char* BillActionInfo::m_name[BillActionInfo::fields_count] = {"operator_id", "provider_id", "service_id", "abonent", "category", "content-type", "result_number", "price", "currency", "bill_type"};

void BillActionInfo::init(const SectionParams& params,PropertyObject propertyObject)
{
    m_BillId = 0;
    FieldType ft = CheckParameter(params, propertyObject, "bill:info", "bill_id", true, true, m_sBillId, bExist);
    if(ft == ftUnknown && !(m_BillId = atoi(m_sBillId.c_str())))
        throw InvalidPropertyException("Action 'bill:info': category should be positive integer value");

    for(int i = 0; i < fields_count; i++)
        CheckParameter(params, propertyObject, "bill:info", m_name[i], false, false, m_sField[i], m_exist[i]);

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
                smsc_log_error(logger,"Action 'bill:info' :: Invalid property %s for BillID", m_sBillId.c_str());
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
            smsc_log_error(logger,"Action 'bill:info' :: Invalid property %s for %s", m_sField[i].c_str(), m_name[i]);
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
            case 6: p->setInt(tr.ServiceNumber); break;
            case 7:
                {
                    char buf[20];
                    sprintf(buf, "%.3lf", tr.Price);
                    p->setStr(buf);
                    break;
                }
            case 8: p->setStr(tr.Currency); break;
            case 9: p->setInt(tr.billType); break;
        }
        s += ", ";
        s += m_name[i];
        s += "=" + p->getStr();
    }
    smsc_log_debug(logger, s);
    return true;
}

}}}

