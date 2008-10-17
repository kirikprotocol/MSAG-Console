#ifndef __SCAG_RULE_ENGINE_ACTION_CONCAT2__
#define __SCAG_RULE_ENGINE_ACTION_CONCAT2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionConcat : public Action
{
    ActionConcat(const ActionConcat &);
    std::string strVariable;

    Property::string_type strString;
    
    FieldType m_fStrFieldType;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionConcat()  {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
