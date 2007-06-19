#include "ActionReceipt.h"
#include "scag/re/CommandAdapter.h"
#include "scag/re/CommandBrige.h"
#include "smpp/smpp_structures.h"
#include "scag/transport/smpp/SmppManager.h"

namespace scag { namespace re { namespace actions {

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

    smsc_log_debug(logger,"Action 'smpp:receipt' inited. to=%s from=%s state=%s msg_id=%s dst_sme_id=%s", varTo.c_str(), varFrom.c_str(), varState.c_str(), varMsgId.c_str(), varDstSmeId.c_str());
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

    smsc_log_debug(logger, "Action 'receipt' from=%s, to=%s, st=%d, mid=%s, dst=%s", pa->from.toString().c_str(), pa->to.toString().c_str(), pa->state, pa->msgId.c_str(), pa->dstSmeId.c_str());

    context.getRuleStatus().addAction(ptr.release());

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

