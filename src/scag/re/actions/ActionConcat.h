#ifndef __SCAG_RULE_ENGINE_ACTION_CONCAT__
#define __SCAG_RULE_ENGINE_ACTION_CONCAT__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionConcat : public Action
{
    ActionConcat(const ActionConcat &);
    std::string strVariable;

    std::string strString;
    
    FieldType m_fStrFieldType;
    Logger * logger;
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
