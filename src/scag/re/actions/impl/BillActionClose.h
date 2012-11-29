#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONCLOSE_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONCLOSE_H

#include "scag/re/base/LongCallAction2.h"
#include "BillAction.h"
#include "scag/re/base/IntField.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionClose : public BillAction, public ActionLongCallInterface
{
public:
    BillActionClose() : BillAction(false), errCode_(*this,"error_code",false,true) {}

protected:

    // long call interface
    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);

    // xml-handler iface
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );

    virtual const char* opname() const {
        return "bill:close";
    }

private:
    bool actionCommit_;
    IntField errCode_;
};

}}}

#endif
