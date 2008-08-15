#ifndef __SCAG_RULE_ACTION_FACTORY2_IF__
#define __SCAG_RULE_ACTION_FACTORY2_IF__

#include <string>
#include "Action2.h"


namespace scag2 {
namespace re {
namespace actions {


class ActionFactory
{
public: 
    virtual Action * CreateAction(const std::string& name) const = 0;
    virtual void registerChild(const ActionFactory * af) = 0;
};

}}}


#endif
