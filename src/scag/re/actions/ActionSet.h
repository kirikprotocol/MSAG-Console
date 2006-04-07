#ifndef __SCAG_RULE_ENGINE_ACTION_SET__
#define __SCAG_RULE_ENGINE_ACTION_SET__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionSet : public Action
{
    ActionSet(const ActionSet &);
    std::string strVariable;

    std::string strValue;
    std::string wstrValue;
    FieldType valueFieldType;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionSet() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
