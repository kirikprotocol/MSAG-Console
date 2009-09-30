#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONINFOTRANSIT_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONINFOTRANSIT_H

#include "scag/re/base/LongCallAction2.h"
#include "scag/re/base/StringField.h"
#include "BillActionPreOpen.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionInfoTransit : public BillActionPreOpen, public ActionLongCallInterface
{
public:
    BillActionInfoTransit();

protected:
    virtual bool RunBeforePostpone( ActionContext& ctx );
    virtual void ContinueRunning( ActionContext& ctx );
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }

    virtual const char* opname() const { return "bill:info-transit"; }

    // virtual bill::BillOpenCallParamsData* postFillParamsData( bill::BillOpenCallParamsData* data, ActionContext& context );
    virtual void postInit(const SectionParams& params, PropertyObject propertyObject );

private:
    StringField totalAmount_;
    StringField chargeThreshold_;
};

}
}
}

#endif
