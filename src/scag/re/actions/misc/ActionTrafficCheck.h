#ifndef __SCAG_RULE_ENGINE_ACTION_TRAFFICK_CHECK__
#define __SCAG_RULE_ENGINE_ACTION_TRAFFICK_CHECK__

#include "scag/re/actions/ActionContext.h"
#include "scag/re/actions/Action.h"
#include <list>


namespace scag { namespace re { namespace actions {

class ActionTrafficCheck : public Action
{
    ActionTrafficCheck(const ActionTrafficCheck&);


    std::string m_sMax;
    FieldType m_ftMax;

    std::list<Action *> Actions;
    CheckTrafficPeriod m_period;
    PropertyObject propertyObject;

    bool StrToPeriod(CheckTrafficPeriod& _period, std::string& str);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject _propertyObject);
    virtual bool run(ActionContext& context);
    ActionTrafficCheck (){};

    virtual ~ActionTrafficCheck();

};



}}}


#endif
