#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H

#include "scag/re/base/LongCallAction2.h"
#include "scag/re/base/StringField.h"
#include "BillActionPreOpen.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionOpen : public BillActionPreOpen, public ActionLongCallInterface
{
public:
    BillActionOpen(bool transit);

protected:
    /// long call interface
    virtual bool RunBeforePostpone( ActionContext& ctx );
    virtual void ContinueRunning( ActionContext& ctx );

    /// xml-handler iface
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }

    virtual bill::BillOpenCallParamsData* postFillParamsData( bill::BillOpenCallParamsData* data, ActionContext& context );
    virtual void postInit(const SectionParams& params, PropertyObject propertyObject );

private:
    virtual void setTariffStatus( ActionContext&   ctx,
                                  const bill::infrastruct::TariffRec* tariff );

    /// return true if transaction successfully registered
    bool processResult( ActionContext& ctx,
                        bill::billid_type     billid,
                        const bill::infrastruct::TariffRec* tariff );

    virtual const char* opname() const {
        return isTransit() ? "bill:open-transit" : "bill:open";
    }

    BillActionOpen();
private:

    // --- input properties

    bool            hasDescription_;
    std::string     descriptionName_;
    FieldType       descriptionType_;

    bool            hasTimeout_;
    std::string     timeoutFieldName_;
    FieldType       timeoutFieldType_;
    int             timeout_;

    // --- output properties

    std::string     resultFieldName_;
    bool            hasResult_;
    StringField     totalAmount_;
    StringField     chargeThreshold_;

};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H */
