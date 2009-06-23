#include <memory>
#include "BillActionOpen.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
#include "scag/util/Time.h"

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
            return false;
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
            return false;
        }
        if( !isCategory_ )
            billingInfoStruct.category = property->getStr().c_str();

        cat = isCategory_ ? int(property->getInt()) :
        bm.getInfrastructure().GetCategoryID( billingInfoStruct.category );
    } else if( !isCategory_ ) {
        billingInfoStruct.category = categoryFieldName_;
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
        bpd->tariffRec = *tariffRec;
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
        // FIXME: how to (should we?) report disabled billing ?
        return false;
    }

    if (tariffRec->getPrice().empty()) {
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%s, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber.c_str(),
                      tariffRec->CategoryId, tariffRec->MediaTypeId);
    }

    if (tariffRec->billType == infrastruct::EWALLET)
    {
        // filling other fields, essential for EWALLET
        if ( !hasAbonent_ || !hasWalletType_ ) {
            smsc_log_warn(logger,"Action '%s' cannot process. Billing type EWALLET must have abonent and walletType", opname());
            setBillingStatus( context, "abonent and walletType missing", false );
            setTariffStatus( context, 0 );
            return false;
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
                return false;
            }
            billingInfoStruct.AbonentNumber = property->getStr().c_str();
        }
        
        // wallet type
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

        // description
        if ( hasDescription_ ) {
            if ( descriptionType_ == ftUnknown ) {
                billingInfoStruct.description = descriptionName_;
            } else {
                Property * property = context.getProperty(descriptionName_);
                if (!property) {
                    setBillingStatus( context, "Invalid property for description", false );
                    setTariffStatus( context, 0 );
                    smsc_log_error(logger,"Action '%s' :: Invalid property %s for description",
                                   opname(), descriptionName_.c_str() );
                    return false;
                }
                billingInfoStruct.description = property->getStr().c_str();
            }
        }

        // externalId
        if ( hasExternalId_ ) {
            if ( externalIdType_ == ftUnknown ) {
                billingInfoStruct.externalId = externalIdName_;
            } else {
                Property * property = context.getProperty(externalIdName_);
                if (!property) {
                    setBillingStatus( context, "Invalid property for externalId", false );
                    setTariffStatus( context, 0 );
                    smsc_log_error(logger,"Action '%s' :: Invalid property %s for externalId",
                                   opname(), externalIdName_.c_str() );
                    return false;
                }
                billingInfoStruct.externalId = property->getStr().c_str();
            }
        } else {
            char buf[100];
            const util::msectime_type currentTime = util::currentTimeMillis();
            snprintf(buf,sizeof(buf),"msag-%llu-%u-%s",
                     static_cast<unsigned long long>(currentTime),
                     billingInfoStruct.serviceId,
                     billingInfoStruct.AbonentNumber.c_str() );
            billingInfoStruct.externalId = buf;
        }

        // timeout
        if ( hasTimeout_ ) {
            if ( timeoutFieldType_ == ftUnknown ) {
                billingInfoStruct.timeout = timeout_;
            } else {
                Property * property = context.getProperty(timeoutFieldName_);
                if (!property) {
                    setBillingStatus( context, "Invalid property for timeout", false );
                    setTariffStatus( context, 0 );
                    smsc_log_error(logger,"Action '%s' :: Invalid property %s for timeout",
                                   opname(), timeoutFieldName_.c_str() );
                    return false;
                }
                billingInfoStruct.timeout = property->getInt();
            }
        }

        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_OPEN;
        EwalletOpenCallParams* eocp = new EwalletOpenCallParams(bpd.release(),&lcmCtx);
        smsc_log_debug(logger,"eocp created @ %p, bocp=%p, lcp=%p",
                       eocp->getOpen(), static_cast<lcm::LongCallParams*>(eocp) );
        lcmCtx.setParams(eocp);
        smsc_log_debug(logger,"lcm ewallet params created: %p", lcmCtx.getParams() );
        return true;
    } else
#ifdef MSAG_INMAN_BILL
    if( tariffRec->billType == infrastruct::INMAN )
    {
        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_OPEN;
        lcmCtx.setParams( new InmanOpenCallParams(bpd.release()) );
        return true;
    } else
#endif
    {
        try 
        {
            InmanOpenCallParams bp(bpd.release());
            int bi = bm.Open( bp );
            processResult( context, bi, bp.tariffRec() );
        }
        catch (SCAGException& e)
        {
            smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                          opname(), e.what());
            setBillingStatus(context, e.what(), false );
            setTariffStatus(context,0);
        }
    }
    return false;
}


void BillActionOpen::ContinueRunning(ActionContext& context)
{
    BillCallParams *bp = (BillCallParams*)context.getSession().getLongCallContext().getParams();
    if (bp->exception.length())
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                      opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), false);
        setTariffStatus(context,0);
        return;
    }
    BillOpenCallParams* bop = bp->getOpen();
    processResult( context, bop->billId(), bop->tariffRec() );
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
        abonentName_ = temp.toString();
    }

    walletTypeType_ = CheckParameter( params, 
                                      propertyObject, 
                                      opname(), "walletType",
                                      false, true,
                                      walletTypeName_,
                                      hasWalletType_ );
    timeoutFieldType_ = CheckParameter( params, 
                                        propertyObject, 
                                        opname(), "timeout",
                                        false, true,
                                        timeoutFieldName_,
                                        hasTimeout_ );
    timeout_ = 0;
    if ( hasTimeout_ && (timeoutFieldType_ == ftUnknown) ) {
        timeout_ = atoi(timeoutFieldName_.c_str());
        if (!timeout_) throw SCAGException("Action '%s': timeout should be integer or variable",
                                           opname() );
    }

    externalIdType_ = CheckParameter( params,
                                      propertyObject,
                                      opname(), "externalId",
                                      false, true,
                                      externalIdName_,
                                      hasExternalId_ );
    descriptionType_ = CheckParameter( params,
                                       propertyObject,
                                       opname(), "description",
                                       false, true,
                                       descriptionName_,
                                       hasDescription_ );

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
            propertyResNum->setStr( tariffRec ? tariffRec->ServiceNumber.c_str() : "");
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
