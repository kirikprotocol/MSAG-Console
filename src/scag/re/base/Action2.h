#ifndef __SCAG_RULE_ENGINE_ACTION2__
#define __SCAG_RULE_ENGINE_ACTION2__

#include "ActionContext2.h"
#include "IParserHandler2.h"
#include "logger/Logger.h"

namespace scag2 {
namespace re {
namespace actions {

using smsc::logger::Logger;

class Action : public IParserHandler
{
protected:
    Logger * logger;
public:
    Action() : logger(0) { logger = Logger::getInstance("re.actions"); };
    virtual ~Action() {};
    virtual void init(const SectionParams& params,PropertyObject propertyObject) = 0;
    virtual bool run(ActionContext& context) = 0;
    static FieldType CheckParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired, bool readOnly, std::string& strParameter, bool& exist);
};


}}}

#endif // __SCAG_RULE_ENGINE_ACTION__

