#ifndef __SCAG_RULE_ENGINE_ACTION_RECEIPT__
#define __SCAG_RULE_ENGINE_ACTION_RECEIPT__

#include "scag/re/actions/Action.h"
#include "scag/transport/smpp/SmppManager.h"

namespace scag { namespace re { namespace actions {

class PostActionReceipt : public PostProcessAction
{
public:
    Address to, from;
    std::string msgId, dstSmeId;
    uint8_t state;

    PostActionReceipt() {}
    void run()
    {
        scag::transport::smpp::SmppManager::Instance().sendReceipt(from, to, state, msgId.c_str(), dstSmeId.c_str());
    }
};

class ActionReceipt : public Action
{
    FieldType ftTo, ftFrom, ftState, ftMsgId, ftDstSmeId;
    std::string varTo, varFrom, varState, varMsgId, varDstSmeId;

    Address toAddr, fromAddr;
    uint8_t state;

    static Hash<uint8_t> stateHash;
    static Hash<uint8_t> ActionReceipt::initHash();
    uint8_t getMsgState(const char* st);
    bool getStrProperty(ActionContext& context, const std::string& str, const char *field_name, std::string& val);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionReceipt() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
