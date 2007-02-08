#ifndef __SCAG_RULE_ENGINE_ACTION_TLV__
#define __SCAG_RULE_ENGINE_ACTION_TLV__

#include "scag/re/actions/Action.h"

namespace scag { namespace re { namespace actions {

enum{
    TLV_SET=1,
    TLV_GET,
    TLV_DEL,
    TLV_EXIST
};

class ActionTLV : public Action
{
    ActionTLV(const ActionTLV &);
    Logger * logger;
    
    int type, tag, val;
    std::string strVar, strTag;
    static Hash<int> namesHash;
    bool byTag;
    FieldType ftVar, ftTag;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionTLV(int t) : type(t) {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    static Hash<int> InitNames();
};



}}}


#endif
