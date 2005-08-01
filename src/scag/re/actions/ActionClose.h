#ifndef __SCAG_RULE_ENGINE_ACTION_CLOSE__
#define __SCAG_RULE_ENGINE_ACTION_CLOSE__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {

class ActionClose : public Action
{
    ActionClose(const ActionClose&);

    std::string propertyName;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params);
    virtual bool run(ActionContext& context);
    ActionClose (){};

    virtual ~ActionClose();

};



}}}


#endif

