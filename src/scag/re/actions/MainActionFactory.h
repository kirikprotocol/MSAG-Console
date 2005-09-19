#ifndef _SCAG_RULE_MAIN_ACTION_FACTORY_
#define _SCAG_RULE_MAIN_ACTION_FACTORY_

#include "ActionFactory.h"
#include <list>

namespace scag { namespace re { namespace actions {

class MainActionFactory : public ActionFactory
{
    std::list<const ActionFactory*> ChildFactories; 
public:
    Action * CreateAction(const std::string& name) const;
    void registerChild(const ActionFactory * af);
};


}}}


#endif
