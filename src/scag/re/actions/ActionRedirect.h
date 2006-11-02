#ifndef __SCAG_RULE_ENGINE_ACTION_REDIRECT__
#define __SCAG_RULE_ENGINE_ACTION_REDIRECT__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionRedirect : public Action
{
    ActionRedirect(const ActionRedirect &);

    std::string strOA;
    std::string strDA;
    
    FieldType m_fOAFieldType;
    FieldType m_fDAFieldType;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionRedirect()  {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
