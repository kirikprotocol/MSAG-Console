#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONCHECK_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONCHECK_H

#include "scag/re/base/LongCallAction2.h"
#include "BillActionPreOpen.h"
#include "scag/re/base/StringField.h"
#include "scag/re/base/IntField.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionCheck : public BillActionPreOpen, public ActionLongCallInterface
{
public:
    BillActionCheck() : BillActionPreOpen(true),
    transId_(*this,"transId",false,true),
    txStatus_(*this,"txStatus",false,false),
    txAmount_(*this,"amount",false,false),
    txEndDate_(*this,"enddate",false,false) {}

protected:
    // long call interface
    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);

    // xml-handler iface
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }
    virtual void postInit( const SectionParams& params,
                           PropertyObject propertyObject );

    virtual const char* opname() const {
        return "bill:check";
    }

private:
    IntField    transId_;
    StringField txStatus_;  // transaction status
    StringField txAmount_;  // transaction amount
    StringField txEndDate_; // transaction enddate
};

}}}

#endif
