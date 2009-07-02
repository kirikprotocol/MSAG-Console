#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONCHECK_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONCHECK_H

#include "scag/re/base/LongCallAction2.h"
#include "BillActionPreOpen.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionCheck : public BillActionPreOpen, public ActionLongCallInterface
{
public:
    BillActionCheck() : BillActionPreOpen(true) {}

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
    bool        hasTransId_;
    std::string ewalletTransName_;
    FieldType   ewalletTransType_;
    uint32_t    ewalletTransId_;
};

}}}

#endif
