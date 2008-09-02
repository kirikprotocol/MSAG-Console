#include "ActionReceipt2.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/re/base/CommandBridge.h"
#include "smpp/smpp_structures.h"
#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {


void PostActionReceipt::run()
{
    transport::smpp::SmppManager::Instance().sendReceipt(from, to, state, msgId.c_str(), dstSmeId.c_str(), netErrCode);
}


Hash<uint8_t> ActionReceipt::stateHash = ActionReceipt::initHash();

Hash<uint8_t> ActionReceipt::initHash()
{
    Hash<uint8_t> h;
    h["ENROUTE"] = 1;
    h["DELIVERED"] = 2;
    h["EXPIRED"] = 3;
    h["DELETED"] = 4;
    h["UNDELIVERABLE"] = 5;
    h["ACCEPTED"] = 6;
    h["UNKNOWN"] = 7;
    h["REJECTED"] = 8;
    return h;
}

uint8_t ActionReceipt::getMsgState(const char* st)
{
    uint8_t *p = stateHash.GetPtr(st);
    return !p ? 0 : *p;
}

void ActionReceipt::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;

    ftTo = CheckParameter(params, propertyObject, "receipt", "to", true, true, varTo, bExist);
    ftFrom = CheckParameter(params, propertyObject, "receipt", "from", true, true, varFrom, bExist);
    try{
        if(ftTo == ftUnknown)
        {
            Address a(varTo.c_str());
            toAddr = a;
        }
        if(ftFrom == ftUnknown)
        {
            Address a(varFrom.c_str()); 
            fromAddr = a;
        }
    }
    catch(Exception& e)
    {
        throw SCAGException("Action 'smpp:receipt': Invalid 'to'(%s) or 'from'(%s) field", varTo.c_str(), varFrom.c_str());
    }

    ftState = CheckParameter(params, propertyObject, "receipt", "state", true, true, varState, bExist);
    if(ftState == ftUnknown && !(state = getMsgState(varState.c_str())))
        throw SCAGException("Action 'smpp:receipt': Invalid 'message state' field: %s", varState.c_str());

    ftMsgId = CheckParameter(params, propertyObject, "receipt", "msg_id", true, true, varMsgId, bExist);
    ftDstSmeId = CheckParameter(params, propertyObject, "receipt", "dst_sme_id", true, true, varDstSmeId, bExist);
    ftNetErrCode = CheckParameter(params, propertyObject, "receipt", "network_error_code", false, true, varNetErrCode, bNetErrCodeExist);
    if(bNetErrCodeExist && ftNetErrCode == ftUnknown)
    {
        char *endptr;
        netErrCode = strtol(varNetErrCode.c_str(), &endptr, 0);
        if(!varNetErrCode.length() || (endptr && *endptr))
            throw SCAGException("Action 'smpp:receipt': Invalid 'network_error_code' field: %s", varNetErrCode.c_str());
    }        

    smsc_log_debug(logger,"Action 'smpp:receipt' inited. to=%s from=%s state=%s msg_id=%s dst_sme_id=%s, netErrCode=%s", varTo.c_str(), varFrom.c_str(), varState.c_str(), varMsgId.c_str(), varDstSmeId.c_str(), varNetErrCode.c_str());
}

bool ActionReceipt::getStrProperty(ActionContext& context, const std::string& str, const char *field_name, std::string& val)
{
    Property * p = context.getProperty(str);
    if(p) 
        val = p->getStr();
    else
        smsc_log_error(logger,"Action 'smpp:receipt': invalid '%s' property '%s'", field_name, str.c_str());
    return p != NULL;
}

bool ActionReceipt::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'smpp:receipt'");

    PostActionReceipt* pa = new PostActionReceipt();
    auto_ptr<PostActionReceipt> ptr(pa);

    pa->from = fromAddr;
    pa->to = toAddr;
    pa->state = state;

    std::string s;

    if(ftTo != ftUnknown)
    {
        if(!getStrProperty(context, varTo, "to", s))
            return true;
        try{
            Address a(s.c_str());
            pa->to = a;
        }
        catch(...)
        {
            smsc_log_error(logger, "Action 'smpp:receipt': Invalid 'to'(%s) field", varTo.c_str());
            return true;
        }

    }
    if(ftFrom != ftUnknown)
    {
        if(!getStrProperty(context, varFrom, "from", s))
            return true;
        try{
            Address a(s.c_str()); 
            pa->from = a;
        }
        catch(...)
        {
            smsc_log_error(logger, "Action 'smpp:receipt': Invalid 'from'(%s) field", varFrom.c_str());
            return true;
        }

    }

    if(ftState != ftUnknown && (!getStrProperty(context, varState, "state", s) || !(pa->state = getMsgState(s.c_str()))))
        return true;

    if(ftMsgId == ftUnknown)
        pa->msgId = varMsgId;
    else if(!getStrProperty(context, varMsgId, "msg_id", pa->msgId))
        return true;

    if(ftDstSmeId == ftUnknown)
        pa->dstSmeId = varDstSmeId;
    else if(!getStrProperty(context, varDstSmeId, "dst_sme_id", pa->dstSmeId))
        return true;
        
    if(bNetErrCodeExist)
    {
        if(ftNetErrCode == ftUnknown)
            pa->netErrCode = netErrCode;
        else
        {
            Property * p = context.getProperty(varNetErrCode);
            if(!p)
            {
                smsc_log_error(logger,"Action 'smpp:receipt': invalid 'network_error_code' property '%s'", varNetErrCode.c_str());
                return true;
            }
            pa->netErrCode = uint32_t(p->getInt());
        }
        pa->netErrCode = (pa->netErrCode & 0xFFFF) | 0x030000; // 3 - GSM
    }

    smsc_log_debug(logger, "Action 'receipt' from=%s, to=%s, st=%d, mid=%s, dst=%s, netErrCode=%d", pa->from.toString().c_str(), pa->to.toString().c_str(), pa->state, pa->msgId.c_str(), pa->dstSmeId.c_str(), pa->netErrCode);

    context.getSession().getLongCallContext().addAction(ptr.release());

    return true;
}

IParserHandler * ActionReceipt::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'receipt' cannot include child objects");
}

bool ActionReceipt::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
