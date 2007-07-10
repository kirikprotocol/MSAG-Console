#ifndef _BATCH_PERS_ACTION_COMMIT_
#define _BATCH_PERS_ACTION_COMMIT_

#include <string>

#include "scag/re/actions/Action.h"
#include "scag/re/actions/LongCallAction.h"
#include "scag/re/actions/pers/PersAction.h"

namespace scag { namespace pers {

using namespace scag::re::actions;
using namespace scag::re;

    class BatchAction : public LongCallAction
    {
    protected:
	    std::vector<PersBatchAction *> actions;
		PropertyObject pobj;
		
        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);
    public:
        BatchAction() {}
        ~BatchAction() {}
        virtual bool RunBeforePostpone(ActionContext& context);
        virtual void ContinueRunning(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}

#endif
