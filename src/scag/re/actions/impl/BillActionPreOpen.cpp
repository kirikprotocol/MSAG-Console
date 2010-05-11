#include <memory>
#include "BillActionPreOpen.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
#include "scag/sessions/base/Session2.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace re {
namespace actions {

using namespace bill;

BillActionPreOpen::BillActionPreOpen( bool transit ) :
BillAction(transit), categoryId_(0), mediaTypeId_(0),
externalId_(*this,"externalId",false,true),
walletType_(*this,"walletType",false,true),
keywords_(*this,"keywords",false,true)
{
}


BillOpenCallParamsData* BillActionPreOpen::makeParamsData( ActionContext& context )
{
    int cat = categoryId_;
    int mt = mediaTypeId_;

    smsc_log_debug( logger,"Run Action '%s', catid=%d mediaid=%d...", opname(), cat, mt );

    if ( ! isTransit() ) {

        // check bill id
        const std::string transId = getTransId( context );
        if ( transId.empty() ) {
            setBillingStatus( context, "no trans-id specified", false );
            setTariffStatus( context, 0 );
            return 0;
        }

        if ( context.getSession().getTransaction( transId.c_str() ) ) {
            // transaction is already active
            setBillingStatus( context, "Transaction already active", false );
            setTariffStatus( context, 0 );
            smsc_log_warn( logger, "Action '%s': transaction '%s' is already found",
                           opname(), transId.c_str() );
            return 0;
        }
    }

    // Statistics& statistics = Statistics::Instance();
    BillingManager& bm = BillingManager::Instance();

    auto_ptr<BillOpenCallParamsData> bpd(new BillOpenCallParamsData());

    BillingInfoStruct& billingInfoStruct = bpd->billingInfoStruct;
    context.getBillingInfoStruct(billingInfoStruct);

    if ( mediaTypeFieldType_ != ftUnknown )
    {
        Property * property = context.getProperty(mediaTypeFieldName_);
        if (!property)
        {
            setBillingStatus( context, "Invalid property for content-type", false );
            setTariffStatus( context, 0 );
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for content-type",
                           opname(), mediaTypeFieldName_.c_str() );
            return 0;
        }
        if (!isContentType_)
            billingInfoStruct.mediaType = property->getStr().c_str();

        mt = isContentType_ ? int(property->getInt()) :
        bm.getInfrastructure().GetMediaTypeID( billingInfoStruct.mediaType );

    } else if ( !isContentType_ ) {
        billingInfoStruct.mediaType = mediaTypeFieldName_;
    }

    if (categoryFieldType_ != ftUnknown)
    {
        Property * property = context.getProperty(categoryFieldName_);
        if (!property) 
        {
            setBillingStatus( context, "Invalid property for category", false );
            setTariffStatus( context, 0 );
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for category",
                           opname(), categoryFieldName_.c_str() );
            return 0;
        }
        if( !isCategory_ )
            billingInfoStruct.category = property->getStr().c_str();

        cat = isCategory_ ? int(property->getInt()) :
        bm.getInfrastructure().GetCategoryID( billingInfoStruct.category );
    } else if( !isCategory_ ) {
        billingInfoStruct.category = categoryFieldName_;
    }

    if (!cat) {
        smsc_log_warn(logger,"Action '%s' cannot process. Category '%s' is empty or not found",
                      opname(), billingInfoStruct.category.c_str() );
        setBillingStatus( context, "empty category", false );
        setTariffStatus( context, 0 );
        return 0;
    } else if ( !mt ) {
        smsc_log_warn(logger,"Action '%s' cannot process. Media-type '%s' is empty or not found",
                      opname(), billingInfoStruct.mediaType.c_str() );
        setBillingStatus( context, "empty media-type", false );
        setTariffStatus( context, 0 );
        return 0;
    }


    TariffRec * tariffRec = 0;
    try {
        tariffRec = context.getTariffRec(cat, mt);
        if (!tariffRec) throw SCAGException("TariffRec is not valid, cat=%d, mt=%u", cat, mt );
        bpd->tariffRec = *tariffRec;
        tariffRec = &(bpd->tariffRec);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: %s", opname(), e.what());
        setBillingStatus( context, e.what(), false );
        setTariffStatus( context, 0 );
        return 0;
    }

    smsc_log_debug(logger,"tariff record is found, type=%d: cat=%d mt=%d",
                   tariffRec->billType, cat,mt);

    if (tariffRec->billType == bill::infrastruct::NONE)
    {
        smsc_log_warn(logger, "Billing disabled for this tariff entry. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d",
                      tariffRec->ServiceNumber.c_str(), tariffRec->CategoryId,
                      tariffRec->MediaTypeId );
        setBillingStatus( context, "billing disabled", false );
        setTariffStatus( context, 0 );
        return 0;
    }

    if (tariffRec->getPrice().empty()) {
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber.c_str(),
                      tariffRec->CategoryId, tariffRec->MediaTypeId);
    }

    if (tariffRec->billType == infrastruct::EWALLET)
    {
        // filling other fields, essential for EWALLET
        if ( !hasAbonent_ ) {
            smsc_log_warn(logger,"Action '%s' cannot process. Billing type EWALLET must have abonent", opname());
            setBillingStatus( context, "abonent is missing", false );
            setTariffStatus( context, 0 );
            return 0;
        }

        // wallet type override
        if ( walletType_.isFound() ) {
            bpd->tariffRec.Currency = walletType_.getValue(context);
            smsc_log_debug(logger,"Action '%s': session wallet type %s",
                           opname(), bpd->tariffRec.Currency.c_str() );
        }

        if ( abonentType_ == ftUnknown ) {
            billingInfoStruct.AbonentNumber = abonentName_;
        } else {
            Property * property = context.getProperty(abonentName_);
            if (!property) {
                setBillingStatus( context, "Invalid property for abonent", false );
                setTariffStatus( context, 0 );
                smsc_log_error(logger,"Action '%s' :: Invalid property %s for abonent",
                               opname(), abonentName_.c_str() );
                return 0;
            }
            billingInfoStruct.AbonentNumber = property->getStr().c_str();
        }
        
        // wallet type -- will be taken from tarif matrix
        /*
        if ( walletTypeType_ == ftUnknown ) {
            billingInfoStruct.walletType = walletTypeName_;
        } else {
            Property * property = context.getProperty(walletTypeName_);
            if (!property) {
                setBillingStatus( context, "Invalid property for walletType", false );
                setTariffStatus( context, 0 );
                smsc_log_error(logger,"Action '%s' :: Invalid property %s for walletType",
                               opname(), walletTypeName_.c_str() );
                return false;
            }
            billingInfoStruct.walletType = property->getStr().c_str();
        }
         */

        // externalId
        if ( externalId_.isFound() ) {
            billingInfoStruct.externalId = externalId_.getValue(context);
        }

    } else {
        // not ewallet
        if ( isTransit() ) {
            throw SCAGException("Action '%s': transit is not allowed for non-ewallet",opname());
        }
    }

    // filling keywords
    if ( keywords_.isFound() ) {
        billingInfoStruct.keywords = keywords_.getValue(context);
    }
    return postFillParamsData( bpd.release(), context );
}


bool BillActionPreOpen::preContinueRunning(ActionContext& context)
{
    BillCallParams *bp = static_cast<BillCallParams*>(context.getSession().getLongCallContext().getParams());
    if ( ! bp->exception.empty() )
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                      opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), bp->getStatus() );
        setTariffStatus(context,0);
        return false;
    }
    return true;
}


void BillActionPreOpen::init( const SectionParams& params,
                              PropertyObject propertyObject )
{
    BillAction::init( params, propertyObject );

    bool bExist;

    // --- input fields

    categoryFieldType_ = CheckParameter( params,
                                         propertyObject,
                                         opname(),
                                         "category",
                                         false,      // required
                                         true,       // readonly
                                         categoryFieldName_,
                                         isCategory_ );
    if ( !isCategory_ )
    {
        categoryFieldType_ = CheckParameter( params,
                                             propertyObject,
                                             opname(),
                                             "category-str",
                                             false,
                                             true, 
                                             categoryFieldName_,
                                             bExist );
        if (!bExist) throw SCAGException( "Action '%s': category or category-str should be present",
                                          opname() );
    }

    if ( categoryFieldType_ == ftUnknown )
    {
        // numerical constant
        categoryId_ = isCategory_ ? atoi(categoryFieldName_.c_str()) :
        BillingManager::Instance().getInfrastructure().GetCategoryID(categoryFieldName_);
        if (!categoryId_) throw SCAGException("Action '%s': category should be integer or variable",
                                              opname() );
    }


    mediaTypeFieldType_ = CheckParameter( params, 
                                          propertyObject, 
                                          opname(), "content-type",
                                          false, true,
                                          mediaTypeFieldName_,
                                          isContentType_ );
    if (!isContentType_)
    {
        mediaTypeFieldType_ = CheckParameter( params,
                                              propertyObject,
                                              opname(),
                                              "content-type-str",
                                              false, true,
                                              mediaTypeFieldName_,
                                              bExist );
        if (!bExist) throw SCAGException("Action '%s': content-type or content-type-str should be present",
                                         opname() );
    }
    if ( mediaTypeFieldType_ == ftUnknown)
    {
        // numerical constant
        mediaTypeId_ = isContentType_ ? atoi(mediaTypeFieldName_.c_str()) :
        BillingManager::Instance().getInfrastructure().GetMediaTypeID(mediaTypeFieldName_);
        if (!mediaTypeId_) throw SCAGException("Action '%s': content-type should be integer or variable",
                                               opname() );
    }

    // --- special input parameters for ewallet

    abonentType_ = CheckParameter( params, 
                                   propertyObject, 
                                   opname(), "abonent",
                                   false, true,
                                   abonentName_,
                                   hasAbonent_ );
    if ( hasAbonent_ && (abonentType_ == ftUnknown) ) {
        // address
        smsc::sms::Address temp(abonentName_.c_str());
        abonentName_ = temp.toString().c_str();
    }

    if (!externalId_.isFound()) externalId_.init(params,propertyObject);
    walletType_.init(params,propertyObject);
    keywords_.init(params,propertyObject);

    postInit( params, propertyObject );
    smsc_log_debug(logger,"Action '%s' init...", opname() );
}

}}}
