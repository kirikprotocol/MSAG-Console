#ifndef _CDR_ACTION_FACTORY_
#define _CDR_ACTION_FACTORY_

#include <scag/re/actions/ActionFactory.h>
#include <scag/re/actions/Action.h>


namespace scag { namespace bill { 

using namespace scag::re::actions;

class CDRActionFactory : public ActionFactory
{
public:
    virtual Action * CreateAction(const std::string& name) const;
    virtual void registerChild(const ActionFactory * af) {}

};

}}

#endif

