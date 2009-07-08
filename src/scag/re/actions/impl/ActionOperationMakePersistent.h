#ifndef _SCAG_RE_ACTIONS_SESS_ACTIONOPERATIONMAKEPERSISTENT_H
#define _SCAG_RE_ACTIONS_SESS_ACTIONOPERATIONMAKEPERSISTENT_H

#include "scag/re/base/Action2.h"


namespace scag2 {
namespace re {
namespace actions {

class ActionOperationMakePersistent : public Action
{
public:
    ActionOperationMakePersistent() {}

    virtual const char* opname() const { return "operation:make_persistent"; }
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );

protected:
    virtual IParserHandler * StartXMLSubSection( const std::string& name,
                                                 const SectionParams& params, 
                                                 const ActionFactory& factory );
    virtual bool FinishXMLSubSection( const std::string& name );

};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_SESS_ACTIONOPERATIONMAKEPERSISTENT_H */
