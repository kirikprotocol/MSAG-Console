#ifndef _SCAG_RE_ACTIONS_PERS_LONGCALLTESTACTION_H
#define _SCAG_RE_ACTIONS_PERS_LONGCALLTESTACTION_H

#include "scag/re/actions/LongCallAction2.h"

namespace scag2 {
namespace re {
namespace actions {

class LongCallTestAction : public LongCallAction
{
public:
    LongCallTestAction();
    virtual ~LongCallTestAction();

    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual IParserHandler * StartXMLSubSection( const std::string& name,
                                                 const SectionParams& params,
                                                 const ActionFactory& factory );
    virtual bool FinishXMLSubSection( const std::string& name );
    virtual bool RunBeforePostpone( ActionContext& context );
    virtual void ContinueRunning( ActionContext& context );

private:

};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_PERS_LONGCALLTESTACTION_H */
