#ifndef __SCAG_RULE_ACTION_FACTORY_IF__
#define __SCAG_RULE_ACTION_FACTORY_IF__

#include <string>

namespace scag { namespace re { namespace actions {
class Action;
}}}

namespace scag { namespace re { 

using namespace std;
using scag::re::actions::Action;

class ActionFactory
{
public: 
    virtual Action * CreateAction(const std::string& name) const = 0;
    virtual void registerChild(ActionFactory& af) = 0;
};

}}


#endif
