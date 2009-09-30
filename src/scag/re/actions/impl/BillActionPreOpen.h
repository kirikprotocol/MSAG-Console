#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONPREOPEN_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONPREOPEN_H

#include "scag/bill/base/BillingManager.h"
#include "scag/re/base/StringField.h"
#include "BillAction.h"

namespace scag2 {
namespace re {
namespace actions {

/// this one is used to have a common place for parsing arguments
/// for bill::open and bill::close-transit
class BillActionPreOpen : public BillAction
{
public:
    BillActionPreOpen( bool transit );

protected:
    // long call interface
    // virtual bool RunBeforePostpone( ActionContext& ctx );
    // virtual void ContinueRunning( ActionContext& ctx );
    bill::BillOpenCallParamsData* makeParamsData( ActionContext& ctx );
    bool preContinueRunning( ActionContext& context );

    virtual bill::BillOpenCallParamsData* postFillParamsData( bill::BillOpenCallParamsData* data, ActionContext& )
    {
        return data;
    }

    /// xml-handler iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual void postInit( const SectionParams& params,
                           PropertyObject propertyObject ) = 0;
    virtual void setTariffStatus( ActionContext& context, const bill::TariffRec* tariffRec ) {}

    void setExternalId( const std::string& eid ) {
        hasExternalId_ = true;
        externalIdName_ = eid;
        externalIdType_ = ftUnknown;
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

    bool            hasExternalId_;
    std::string     externalIdName_;
    FieldType       externalIdType_;

    StringField     walletType_;
};

} // namespace actions
} // namespace re
} // namespace scag2

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONPREOPEN_H */
