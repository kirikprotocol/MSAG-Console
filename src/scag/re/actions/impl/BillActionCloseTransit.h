#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONCLOSETRANSIT_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONCLOSETRANSIT_H

#include "scag/re/base/LongCallAction2.h"
#include "BillActionPreOpen.h"
#include "scag/re/base/StringField.h"
#include "scag/re/base/IntField.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionCloseTransit : public BillActionPreOpen, public ActionLongCallInterface
{
public:
    BillActionCloseTransit() : BillActionPreOpen(true),
    transId_(*this,"transId",false,true),
    amount_(*this,"amount",false,true) {}

protected:
    // long call interface
    virtual bill::BillOpenCallParamsData* postFillParamsData( bill::BillOpenCallParamsData* data, ActionContext& context );
    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);

    // xml-handler iface
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }
    virtual void postInit( const SectionParams& params,
                           PropertyObject propertyObject );

    virtual const char* opname() const {
        return "bill:close-transit";
    }

private:
    bool actionCommit_;

    IntField    transId_;
    // std::string ewalletTransName_;
    // FieldType   ewalletTransType_;
    // uint32_t    ewalletTransId_;

    StringField amount_;
};

}}}

#endif
