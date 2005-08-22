#ifndef __SCAG_RULE_ACTION_FACTORY_IF__
#define __SCAG_RULE_ACTION_FACTORY_IF__

#include <string>
#include "Action.h"


namespace scag { namespace re { namespace actions {


class ActionFactory
{
public: 
    virtual Action * CreateAction(const std::string& name) const = 0;
    virtual void registerChild(const ActionFactory& af) = 0;
};

}}}


#endif
