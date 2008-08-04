#ifndef _SCAG_RULE_MAIN_ACTION_FACTORY2_
#define _SCAG_RULE_MAIN_ACTION_FACTORY2_

#include "ActionFactory2.h"
#include <list>
#include <logger/Logger.h>

namespace scag2 {
namespace re {
namespace actions {

using smsc::logger::Logger;

class MainActionFactory : public ActionFactory
{
    Logger * logger;
    std::list<const ActionFactory*> ChildFactories; 
public:
    MainActionFactory() : logger(0) {logger = Logger::getInstance("scag.re");};
    Action * CreateAction(const std::string& name) const;
    void registerChild(const ActionFactory * af);
};


}}}


#endif
