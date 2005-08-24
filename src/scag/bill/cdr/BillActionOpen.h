#ifndef _BILL_ACTION_OPEN_
#define _BILL_ACTION_OPEN_

#include "BillAction.h"
#include <string>

namespace scag { namespace bill {

using namespace scag::re::actions;

    class BillActionOpen : public BillAction
    {
    protected:
        std::string m_sName;
    public:
        BillActionOpen() : m_sName("bill:open") {}
        virtual bool run(ActionContext& context);
    };

}}


#endif


