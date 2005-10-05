#ifndef __SCAG_RULE_ENGINE_ACTION_LOG__
#define __SCAG_RULE_ENGINE_ACTION_LOG__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {

class ActionLog : public Action
{
    ActionLog(const ActionLog&);

    std::string sCategory;
    std::string msg;


    enum LogLevel
    {
        lgWarning,
        lgError,
        lgDebug,
        lgInfo
    };

    LogLevel level;
    FieldType ftCategory,ftMessage;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionLog (){};

    virtual ~ActionLog();

};


}}}


#endif

