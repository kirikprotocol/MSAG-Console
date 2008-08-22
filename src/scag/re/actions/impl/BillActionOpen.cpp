#include <memory>
#include "BillActionOpen.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"

namespace scag2 {
namespace re {
namespace actions {

using namespace bill;

BillActionOpen::BillActionOpen() :
categoryId_(0),mediaTypeId_(0)
{
}


bool BillActionOpen::RunBeforePostpone( ActionContext& context )
{
    int cat = categoryId_;
    int mt = mediaTypeId_;

    smsc_log_debug( logger,"Run Action '%s', catid=%d mediaid=%d...", opname(), cat, mt );

    // check bill id
    const std::string transId = getTransId( context );
    if ( transId.empty() ) {
        setTariffStatus( context, 0 );
        return false;
    }

    if ( context.getSession().getTransaction( transId.c_str() ) ) {
        // transaction is already active
        setBillingStatus( context, "Transaction already active", false );
        setTariffStatus( context, 0 );
        smsc_log_warn( logger, "Action '%s': transaction '%s' is already found",
                       opname(), transId.c_str() );
        return false;
    }

    // Statistics& statistics = Statistics::Instance();
    BillingManager& bm = BillingManager::Instance();

    auto_ptr<BillOpenCallParams> bp(new BillOpenCallParams());

    context.getBillingInfoStruct(bp->billingInfoStruct);

    if ( mediaTypeFieldType_ != ftUnknown )
    {
        Property * property = context.getProperty(mediaTypeFieldName_);
        if (!property)
        {
            setBillingStatus( context, "Invalid property for content-type", false );
            setTariffStatus( context, 0 );
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for content-type",
                           opname(), mediaTypeFieldName_.c_str() );
            return false;
        }
        if (!isContentType_)
            bp->billingInfoStruct.mediaType = property->getStr();

        mt = isContentType_ ? property->getInt() :
        bm.getInfrastructure().GetMediaTypeID( bp->billingInfoStruct.mediaType );

    } else if ( !isContentType_ ) {
        bp->billingInfoStruct.mediaType = mediaTypeFieldName_;
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
            return false;
        }
        if( !isCategory_ )
            bp->billingInfoStruct.category = property->getStr();

        cat = isCategory_ ? property->getInt() :
        bm.getInfrastructure().GetCategoryID( bp->billingInfoStruct.category );
    } else if( !isCategory_ ) {
        bp->billingInfoStruct.category = categoryFieldName_;
    }

    if (!cat || !mt)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Empty category or content-type",
                      opname() );
        return false;
    }

    TariffRec * tariffRec = 0;
    try {
        tariffRec = context.getTariffRec(cat, mt);
        if (!tariffRec) throw SCAGException("TariffRec is not valid, cat=%d, mt=%u", cat, mt );
        bp->tariffRec = *tariffRec;
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: %s", opname(), e.what());
        setBillingStatus( context, e.what(), false );
        setTariffStatus( context, 0 );
        return false;
    }

    if (tariffRec->billType == bill::infrastruct::NONE)
    {
        smsc_log_warn(logger, "Billing desabled for this tariff entry. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d",
                      tariffRec->ServiceNumber.c_str(), tariffRec->CategoryId,
                      tariffRec->MediaTypeId );
        // FIXME: how to report disabled billing ?
        return false;
    }

    if (tariffRec->Price == 0)
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber.c_str(), tariffRec->CategoryId, tariffRec->MediaTypeId);

#ifdef MSAG_INMAN_BILL
    if( tariffRec->billType == infrastruct::INMAN )
    {
        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_OPEN;
        lcmCtx.setParams( bp.release() );
        return true;
    }
    else
    {
#endif
        try 
        {
            int bi = bm.Open(bp->billingInfoStruct, bp->tariffRec);
            processResult( context, bi, &bp->tariffRec );
        }
        catch (SCAGException& e)
        {
            smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                          opname(), e.what());
            setBillingStatus(context, e.what(), false );
            setTariffStatus(context,0);
        }
#ifdef MSAG_INMAN_BILL
    }
#endif
    return false;
}


void BillActionOpen::ContinueRunning(ActionContext& context)
{
    BillOpenCallParams *bp = (BillOpenCallParams*)context.getSession().getLongCallContext().getParams();
    if (bp->exception.length())
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                      opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), false);
        setTariffStatus(context,0);
        return;
    }
    processResult( context, bp->BillId, &bp->tariffRec );
}


void BillActionOpen::init( const SectionParams& params,
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

    // --- output fields

    CheckParameter( params,
                    propertyObject,
                    opname(), "result_number",
                    false, false,
                    resultFieldName_,
                    hasResult_ );

    // if (m_waitOperation) InitParameters(params, propertyObject, logger);
    smsc_log_debug(logger,"Action '%s' init...", opname() );
}


void BillActionOpen::setTariffStatus( ActionContext&   context,
                                      const TariffRec* tariffRec )
{
    if (hasResult_) 
    {
        Property * propertyResNum = context.getProperty(resultFieldName_);

        if (!propertyResNum)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for result_number",
                           opname(), resultFieldName_.c_str());
        else
            propertyResNum->setStr( tariffRec ? tariffRec->ServiceNumber : "");
    }
}


void BillActionOpen::processResult( ActionContext& context,
                                    int billId, 
                                    const infrastruct::TariffRec* tariffRec)
{
    std::string transId = getTransId( context );
    smsc_log_debug( logger, "Action '%s': process result trans-id='%s' billid=%d", 
                    opname(), transId.c_str(), billId );
    std::auto_ptr<ExternalTransaction> trans( new sessions::ExternalBillingTransaction( billId ) );
    if ( ! context.getSession().addTransaction( transId.c_str(), trans ) ) {
        smsc_log_error( logger, "Action '%s': cannot put transaction '%s' in session",
                        opname(), transId.c_str() );
        setBillingStatus( context, "Session::addTranaction failed", false);
        setTariffStatus( context, 0 );
        return;
    }


    /*
    Operation * operation = context.getSession().getCurrentOperation();
    if (!operation)
    {
        smsc_log_error(logger,"Action '%s': Fatal error in action - operation from ActionContext is invalid", opname );
        setBillingStatus(context, "operation is invalid", false, 0);
        return;
    }

    try 
    {
        if (m_waitOperation) 
            RegisterPending(context, logger, billId);
        else
            operation->attachBill(billId);
    }
    catch (SCAGException& e)
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false, 0);
        if (!m_waitOperation) operation->detachBill();

        BillingManager::Instance().Rollback(billId);
        return;
    }

    if(m_BillIdExist) 
    {
        Property * p = context.getProperty(m_sBillId);

        if (!p)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for BillID", m_ActionName.c_str(), m_sBillId.c_str());
        else
            p->setInt(billId);
    }
     */

    setBillingStatus( context, "", true );
    setTariffStatus( context, tariffRec );
    smsc_log_debug( logger, "Action '%s' transaction '%s' successfully opened (billId=%d)",
                    opname(), transId.c_str(), billId );
    return;
}

}}}
