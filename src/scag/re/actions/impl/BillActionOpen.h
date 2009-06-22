#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H

#include "scag/re/base/LongCallAction2.h"
#include "scag/bill/base/Infrastructure.h"
#include "BillAction.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionOpen : public BillAction, ActionLongCallInterface
{
public:
    BillActionOpen();

protected:
    /// long call interface
    virtual bool RunBeforePostpone( ActionContext& ctx );
    virtual void ContinueRunning( ActionContext& ctx );

    /// xml-handler iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual bool run( ActionContext& ctx ) {
        return dorun( ctx );
    }

private:
    void setTariffStatus( ActionContext&   ctx,
                          const bill::infrastruct::TariffRec* tariff );

    void processResult( ActionContext& ctx,
                        int            billid,
                        const bill::infrastruct::TariffRec* tariff );

    virtual const char* opname() const {
        return "bill:open";
    }

private:

    // --- input properties

    bool            isCategory_;        // flag: true=category, false=category-str
    std::string     categoryFieldName_; // the name of category or category-str parameter
    FieldType       categoryFieldType_; // the type of the field
    int             categoryId_;        // the id of the category
    
    bool            isContentType_;
    std::string     mediaTypeFieldName_;
    FieldType       mediaTypeFieldType_;
    int             mediaTypeId_;
    
    bool            hasAbonent_;
    std::string     abonentName_;
    FieldType       abonentType_;

    bool            hasWalletType_;
    std::string     walletTypeName_;
    FieldType       walletTypeType_;

    bool            hasDescription_;
    std::string     descriptionName_;
    FieldType       descriptionType_;

    bool            hasExternalId_;
    std::string     externalIdName_;
    FieldType       externalIdType_;

    bool            hasTimeout_;
    std::string     timeoutFieldName_;
    FieldType       timeoutFieldType_;
    int             timeout_;

    // --- output properties

    std::string     resultFieldName_;
    bool            hasResult_;


};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONOPEN_H */
