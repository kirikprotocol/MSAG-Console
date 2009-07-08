#ifndef __SCAG_RULE_ENGINE_ACTION_SET2__
#define __SCAG_RULE_ENGINE_ACTION_SET2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionSet : public Action
{
    ActionSet(const ActionSet &);
    std::string m_strVariable;

    Property::string_type m_strValue;
    FieldType m_valueFieldType;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual const char* opname() const { return "set"; }
    virtual bool run(ActionContext& context);

    ActionSet() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};


}}}

#endif
