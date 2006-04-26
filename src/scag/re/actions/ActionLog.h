#ifndef __SCAG_RULE_ENGINE_ACTION_LOG__
#define __SCAG_RULE_ENGINE_ACTION_LOG__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {

class ActionLog : public Action
{
public:
    enum LogLevel
    {
        lgWarning,
        lgError,
        lgDebug,
        lgInfo
    };


    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionLog (LogLevel l){ level = l; }

    virtual ~ActionLog();

protected:
    ActionLog(const ActionLog&);

    std::string wstrCategory;
    std::string wstrMsg;

    std::string strCategory;
    std::string strMsg;


    LogLevel level;
    FieldType ftCategory,ftMessage;

    std::string ttToStr(TransportType t);

    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
};


}}}


#endif

