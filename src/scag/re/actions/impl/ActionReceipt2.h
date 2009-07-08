#ifndef __SCAG_RULE_ENGINE_ACTION_RECEIPT2__
#define __SCAG_RULE_ENGINE_ACTION_RECEIPT2__

#include "scag/re/base/Action2.h"
#include "scag/re/base/LongCallContext.h"
#include "sms/sms.h"

namespace scag2 {
namespace re {
namespace actions {

class PostActionReceipt : public lcm::PostProcessAction
{
public:
    smsc::sms::Address to, from;
    std::string msgId, dstSmeId;
    uint8_t state;
    uint32_t netErrCode;

    PostActionReceipt() : netErrCode(0) {}
    void run();
};

class ActionReceipt : public Action
{
    FieldType ftTo, ftFrom, ftState, ftMsgId, ftDstSmeId, ftNetErrCode;
    std::string varTo, varFrom, varState, varMsgId, varDstSmeId, varNetErrCode;
    bool bNetErrCodeExist;
    uint32_t netErrCode;

    smsc::sms::Address toAddr, fromAddr;
    uint8_t state;

    static Hash<uint8_t> stateHash;
    static Hash<uint8_t> initHash();
    uint8_t getMsgState(const char* st);
    bool getStrProperty(ActionContext& context, const std::string& str, const char *field_name, std::string& val);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionReceipt() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual const char* opname() const { return "smpp:receipt"; }
};



}}}


#endif
