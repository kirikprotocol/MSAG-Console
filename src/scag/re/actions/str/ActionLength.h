#ifndef __SCAG_RULE_ENGINE_ACTION_LENGTH__
#define __SCAG_RULE_ENGINE_ACTION_LENGTH__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionLength : public Action
{
    ActionLength(const ActionLength &);

    std::auto_ptr<ActionParameter> paramVar, paramResult;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionLength() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
