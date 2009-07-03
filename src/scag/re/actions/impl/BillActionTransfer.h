#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONTRANSFER_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONTRANSFER_H

#include "scag/bill/base/BillingManager.h"
#include "scag/re/base/LongCallAction2.h"
#include "BillAction.h"

namespace scag2 {
namespace re {
namespace actions {

// transfer funds to another wallet
class BillActionTransfer : public BillAction, public ActionLongCallInterface
{
public:
    BillActionTransfer() : BillAction(true) {}

protected:
    // long call interface
    virtual void init( const SectionParams& params,
                       PropertyObject       propertyObject );
    virtual bool RunBeforePostpone( ActionContext& ctx );
    virtual void ContinueRunning( ActionContext& ctx );

    virtual const char* opname() const {
        return "bill:transfer";
    }
    virtual bool run( ActionContext& ctx ) {
        return dorun(ctx);
    }

protected:
    std::string getString( ActionContext& ctx,
                           const char* fieldName,
                           FieldType fieldType,
                           const std::string& fieldValue ); // throw

    int getInt( ActionContext& ctx,
                const char* fieldName,
                FieldType fieldType,
                const std::string& fieldValueStr,
                int fieldValueInt ); // throw

private:
    std::string     abonentName_;
    FieldType       abonentType_;

    std::string     srcWalletName_;
    FieldType       srcWalletType_;

    std::string     dstWalletName_;
    FieldType       dstWalletType_;

    bool            hasExternalId_;
    std::string     externalIdName_;
    FieldType       externalIdType_;

    std::string     amountName_;
    FieldType       amountType_;
    int32_t         amount_;

    bool            hasDescription_;
    std::string     descriptionName_;
    FieldType       descriptionType_;
};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONTRANSFER_H */
