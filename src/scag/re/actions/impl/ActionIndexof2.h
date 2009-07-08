#ifndef __SCAG_RULE_ENGINE_ACTION_INDEXOF2__
#define __SCAG_RULE_ENGINE_ACTION_INDEXOF2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

// using namespace re::actions;

class ActionIndexof : public Action
{
    ActionIndexof(const ActionIndexof &);
    Property::string_type strVariable;
    std::string strResult;
    Property::string_type strString;

    FieldType m_fVariableFieldType;
    FieldType m_fStringFieldType;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionIndexof() {};
    virtual const char* opname() const { return "strings:indexof"; }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};


}}}

#endif
