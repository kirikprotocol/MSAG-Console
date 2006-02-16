#ifndef _BILL_ACTION_COMMIT_
#define _BILL_ACTION_COMMIT_

#include "BillAction.h"
#include <string>

namespace scag { namespace bill {

using namespace scag::re::actions;

    class BillActionCommit : public BillAction
    {
    protected:
        std::string m_sName;
    public:
        BillActionCommit() : m_sName("bill:commit") {}
        virtual bool run(ActionContext& context);
    };

}}


#endif


