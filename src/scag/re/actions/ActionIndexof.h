#ifndef __SCAG_RULE_ENGINE_ACTION_INDEXOF__
#define __SCAG_RULE_ENGINE_ACTION_INDEXOF__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionIndexof : public Action
{
    ActionIndexof(const ActionIndexof &);
    std::string strVariable;

    std::string strResult;
    
    std::string strString;

    FieldType m_fVariableFieldType;
    FieldType m_fStringFieldType;
    Logger * logger;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionIndexof() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif
