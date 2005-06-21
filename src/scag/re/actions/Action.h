#ifndef SCAG_RULE_ENGINE_ACTION
#define SCAG_RULE_ENGINE_ACTION

#include "ActionContext.h"

namespace scag { namespace re { namespace actions 
{
    /**
     * Abstract action interface to run on ActionContext
     */
    struct Action
    {
        virtual void run(ActionContext& context) = 0;
        virtual ~Action() {};

    protected:

        Action() {};
    };

}}}

#endif // SCAG_RULE_ENGINE_ACTION

