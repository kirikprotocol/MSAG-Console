#ifndef _SCAG_RE_ACTIONS_SESS_ACTIONSESSIONCONTEXTSCOPE_H
#define _SCAG_RE_ACTIONS_SESS_ACTIONSESSIONCONTEXTSCOPE_H

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionSessionContextScope : public Action
{
public:
    enum ActionType {
        NEW = 0,
        SET,
        DEL
    };
    
public:
    ActionSessionContextScope( ActionType tp );
    virtual void init( const SectionParams& params, PropertyObject propertyObject );
    virtual bool run( ActionContext& context );

protected:
    virtual IParserHandler * StartXMLSubSection( const std::string& name,
                                                 const SectionParams& params, 
                                                 const ActionFactory& factory );
    virtual bool FinishXMLSubSection( const std::string& name );

private:
    const char* actionname() const;

private:
    ActionType   type_;
    std::string  idfieldname_;  // name of variable to get/set scope id into
};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_SESS_ACTIONSESSIONCONTEXTSCOPE_H */
