#ifndef __SCAG_RULE_ENGINE_ACTION_LOG2__
#define __SCAG_RULE_ENGINE_ACTION_LOG2__

#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/Action2.h"


namespace scag2 {
namespace re {
namespace actions {

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

    virtual const char* opname() const { return opname_.c_str(); }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionLog (LogLevel l) {
        level = l; 
        switch (l) {
        case lgWarning : opname_ = "log:warn"; break;
        case lgError   : opname_ = "log:error"; break;
        case lgDebug   : opname_ = "log:debug"; break;
        case lgInfo    : opname_ = "log:info"; break;
        default : opname_ = "???";
        }
    }

    virtual ~ActionLog();

protected:
    ActionLog(const ActionLog&);

    std::string strCategory;
    std::string strMsg;

    LogLevel level;
    std::string opname_;
    FieldType ftCategory,ftMessage;

    std::string ttToStr(TransportType t);

    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
};


}}}


#endif

