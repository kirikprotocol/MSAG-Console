#ifndef _SCAG_RE_ACTIONS_IMPL_ACTIONSESSIONWAIT_H
#define _SCAG_RE_ACTIONS_IMPL_ACTIONSESSIONWAIT_H

#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionSessionWait : public Action
{
public:
    ActionSessionWait();

protected:
    // action iface
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );
    virtual IParserHandler * StartXMLSubSection( const std::string& name,
                                                 const SectionParams& params, 
                                                 const ActionFactory& factory );
    virtual bool FinishXMLSubSection( const std::string& name );

    FieldType   waitFieldType_;
    std::string waitFieldName_;
    unsigned    wait_;
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_ACTIONSESSIONWAIT_H */
