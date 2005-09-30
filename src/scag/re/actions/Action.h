#ifndef __SCAG_RULE_ENGINE_ACTION__
#define __SCAG_RULE_ENGINE_ACTION__

#include "ActionContext.h"
#include <scag/re/actions/IParserHandler.h>
#include <logger/Logger.h>


namespace scag { namespace re { namespace actions 
{

using smsc::logger::Logger;

class Action : public IParserHandler
{
protected:
    Logger * logger;
public:
    Action() : logger(0) {};
    virtual bool run(ActionContext& context) = 0;
    virtual void init(const SectionParams& params,PropertyObject propertyObject) = 0;
};

}}}

#endif // __SCAG_RULE_ENGINE_ACTION__

