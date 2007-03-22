#ifndef __SCAG_RULE_ENGINE_ACTION_RECEIPT__
#define __SCAG_RULE_ENGINE_ACTION_RECEIPT__

#include "scag/re/actions/Action.h"

namespace scag { namespace re { namespace actions {

class ActionReceipt : public Action
{
    FieldType ftTo, ftFrom, ftState, ftMsgId, ftSrcSmeId, ftDstSmeId;
    std::string varTo, varFrom, varState, varMsgId, varSrcSmeId, varDstSmeId, msgId, srcSmeId, dstSmeId;

    Address toAddr, fromAddr;
    uint8_t state;

    static Hash<uint8_t> stateHash;
    static Hash<uint8_t> ActionReceipt::initHash();
    uint8_t getMsgState(const char* st);
    bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
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
