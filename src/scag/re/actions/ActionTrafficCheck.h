#ifndef __SCAG_RULE_ENGINE_ACTION_TRAFFICK_CHECK__
#define __SCAG_RULE_ENGINE_ACTION_TRAFFICK_CHECK__

#include "ActionContext.h"
#include "Action.h"
#include <list>


namespace scag { namespace re { namespace actions {

class ActionTrafficCheck : public Action
{
    ActionTrafficCheck(const ActionTrafficCheck&);


    std::string sPeriod;
    std::string sMax;
    std::list<Action *> Actions;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params);
    virtual bool run(ActionContext& context);
    ActionTrafficCheck (){};

    virtual ~ActionTrafficCheck();

};



}}}


#endif

