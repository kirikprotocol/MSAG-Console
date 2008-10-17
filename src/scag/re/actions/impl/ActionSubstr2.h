#ifndef __SCAG_RULE_ENGINE_ACTION_SUBSTR2__
#define __SCAG_RULE_ENGINE_ACTION_SUBSTR2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionSubstr : public Action
{
    ActionSubstr(const ActionSubstr &);
    Property::string_type m_strVariable;
    std::string m_strResult;
    std::string m_strBegin;
    std::string m_strEnd;
    
    int beginIndex;
    int endIndex;
    FieldType m_fVariableFieldType;
    FieldType m_ftBegin;
    FieldType m_ftEnd;

    bool m_bExistBegin;
    bool m_bExistEnd;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionSubstr() : beginIndex(-1), endIndex(-1) {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}

#endif
