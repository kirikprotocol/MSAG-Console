#ifndef __SCAG_RULE_ENGINE_ACTION_LENGTH__
#define __SCAG_RULE_ENGINE_ACTION_LENGTH__

#include "scag/re/actions/Action.h"

namespace scag { namespace re { namespace actions {

class ActionLength : public Action
{
    ActionLength(const ActionLength &);

    std::string m_strVar, m_strResult;
    FieldType   m_varFieldType;

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
