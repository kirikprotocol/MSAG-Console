#ifndef _SCAG_RE_ACTIONS_SESS_ACTIONSESSIONDESTROYSERVICE_H
#define _SCAG_RE_ACTIONS_SESS_ACTIONSESSIONDESTROYSERVICE_H

#include "scag/re/base/Action2.h"
#include "scag/re/base/TimeField.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionSessionDestroyService : public Action
{
public:
    ActionSessionDestroyService() : wait_(*this,"wait",false,true) {}

    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );
    virtual const char* opname() const { return "session:destroy_service"; }

protected:
    virtual IParserHandler * StartXMLSubSection( const std::string& name,
                                                 const SectionParams& params, 
                                                 const ActionFactory& factory );
    virtual bool FinishXMLSubSection( const std::string& name );

protected:
    TimeField wait_;
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_SESS_ACTIONSESSIONDESTROYSERVICE_H */
