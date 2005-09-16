#include "CDRActionFactory.h"
#include "BillActionOpen.h"
#include "BillActionOpen.h"


namespace scag { namespace bill {

Action * CDRActionFactory::CreateAction(const std::string& name) const
{
    if (name == "bill:open") return new BillActionOpen();

    return 0;
}



}}
