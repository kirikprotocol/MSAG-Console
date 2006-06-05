#ifndef __SCAG_RULE_ENGINE_ACTION_SUBSTR__
#define __SCAG_RULE_ENGINE_ACTION_SUBSTR__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionSubstr : public Action
{
    ActionSubstr(const ActionSubstr &);
    std::string m_strVariable;

    std::string m_strResult;
    
    int beginIndex;
    int endIndex;
    FieldType m_fVariableFieldType;
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
