#ifndef _SCAG_RULE_MAIN_ACTION_FACTORY_
#define _SCAG_RULE_MAIN_ACTION_FACTORY_

#include "ActionFactory.h"

namespace scag { namespace re {

class MainActionFactory : public ActionFactory
{
public:
    Action * CreateAction(const std::string& name) const;
    void registerChild(ActionFactory& af);
};


}}



#endif
