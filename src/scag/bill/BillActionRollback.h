#ifndef _BILL_ACTION_ROLLBACK_
#define _BILL_ACTION_ROLLBACK_

#include "BillAction.h"
#include <string>

namespace scag { namespace bill {

using namespace scag::re::actions;

    class BillActionRollback : public BillAction
    {
    protected:
        std::string m_sName;
    public:
        BillActionRollback() : m_sName("bill:rollback") {}
        virtual bool run(ActionContext& context);
    };

}}


#endif


