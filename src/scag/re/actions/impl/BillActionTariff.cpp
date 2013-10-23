#include "BillActionTariff.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace re {
namespace actions {

BillActionTariff::BillActionTariff() :
BillAction(true),
category_(*this,"category",false,true),
categoryStr_(*this,"category-str",false,true),
contentType_(*this,"content-type",false,true),
contentTypeStr_(*this,"content-type-str",false,true),
operator_(*this,"operator-id",false,true),
operatorId_(0),
catId_(0),
contId_(0),
catField_(0),
contField_(0),
resultServiceNumber_(*this,"result-service-number",false,false),
resultCurrency_(*this,"result-currency",false,false),
resultMediaType_(*this,"result-media-type",false,false),
resultCategory_(*this,"result-category",false,false),
resultBillType_(*this,"result-bill-type",false,false),
resultPrice_(*this,"result-price",false,false)
{
}


void BillActionTariff::init( const SectionParams& params,
                             PropertyObject propertyObject )
{
    BillAction::init(params,propertyObject);

    category_.init(params,propertyObject);
    catField_ = &category_;
    if ( ! category_.isFound() ) {
        categoryStr_.init(params,propertyObject);
        if ( ! categoryStr_.isFound() ) {
            throw SCAGException( "Action %s: category or category-str should be specified",opname());
        }
        catField_ = &categoryStr_;
    }

    if ( catField_->getType() == ftUnknown ) {
        catId_ = ( catField_ == &category_ )
            ? (int)category_.getIntValue()
            : bill::BillingManager::Instance().getInfrastructure().GetCategoryID(categoryStr_.getStringValue());
        if ( ! catId_ ) {
            throw SCAGException("Action %s: category should be int or variable", opname());
        }
    }
    
    contentType_.init(params,propertyObject);
    contField_ = &contentType_;
    if ( ! contentType_.isFound() ) {
        contentTypeStr_.init(params,propertyObject);
        if ( ! contentTypeStr_.isFound() ) {
            throw SCAGException( "Action %s: content-type or content-type-str should be specified",opname());
        }
        contField_ = &contentTypeStr_;
    }

    if ( contField_->getType() == ftUnknown ) {
        contId_ = contField_ == &contentType_ ?
            (int)contentType_.getIntValue() : bill::BillingManager::Instance()
                .getInfrastructure().GetMediaTypeID(contentTypeStr_.getStringValue());
        if ( ! contId_ ) {
            throw SCAGException("Action %s: content-type should be int or variable", opname());
        }
    }

// optional operator_id
    if ( operator_.init(params,propertyObject) )
      operatorId_ = (int)operator_.getIntValue();
    else
      operatorId_ = 0;
//
    resultServiceNumber_.init(params,propertyObject);
    resultCurrency_.init(params,propertyObject);
    resultMediaType_.init(params,propertyObject);
    resultCategory_.init(params,propertyObject);
    resultBillType_.init(params,propertyObject);
    resultPrice_.init(params,propertyObject);

    smsc_log_debug(logger,"Action %s init. Optional operator-id=%d", opname(), operatorId_);
}


bool BillActionTariff::run( ActionContext& context )
{
    bill::BillingManager& bm = bill::BillingManager::Instance();
    int cat = catId_;
    int mt = contId_;
    int op = operatorId_;
    if ( contField_->getType() != ftUnknown ) {
        if ( contField_ == &contentType_ ) {
            mt = (int)contentType_.getValue( context );
        }
        else {
            mt = bm.getInfrastructure().GetMediaTypeID( contentTypeStr_.getValue(context));
        }
    }
    if ( catField_->getType() != ftUnknown ) {
        if ( catField_ == &category_ ) {
            cat = (int)category_.getValue( context );
        }
        else {
            cat = bm.getInfrastructure().GetCategoryID( categoryStr_.getValue(context) );
        }
    }
    
    if ( !cat || !mt ) {
        smsc_log_warn(logger,"Action %s: empty cat or mt", opname());
        setBillingStatus(context,"empty cat/mt", false);
        return true;
    }

    smsc_log_debug(logger,"%d tariff record (%d,%d,%d) search...", opname(), cat, mt, op);
    bill::TariffRec* tariffRec = 0;
    try
    {
        tariffRec = context.getTariffRec(cat, mt, op);
        if ( ! tariffRec ) {
          smsc_log_debug(logger, "wrong tariff rec, cat=%d mt=%d operator-id=%d", cat, mt, op);
          throw SCAGException("wrong tariff rec, cat=%d mt=%d operator-id=%d", cat, mt, op);
        }
    }
    catch ( std::exception& e )
    {
        smsc_log_warn(logger,"Action %s: exc %s", opname(), e.what() );
        setBillingStatus(context, e.what(), false );
        return true;
    }

    smsc_log_debug(logger,"%d tariff record (%d,%d,%d) found: svc=%s currency=%s catid=%d mediaid=%d billType=%d price=%s",
       opname(), cat, mt, op,
       tariffRec->ServiceNumber.c_str(),
       tariffRec->Currency.c_str(),
       tariffRec->CategoryId,
       tariffRec->MediaTypeId,
       tariffRec->billType,
       tariffRec->getPrice().c_str() );

    if ( resultServiceNumber_.isFound() ) {
        Property* p = resultServiceNumber_.getProperty(context);
        if (p) p->setStr(tariffRec->ServiceNumber.c_str());
    }
    if ( resultCurrency_.isFound() ) {
        Property* p = resultCurrency_.getProperty(context);
        if (p) p->setStr(tariffRec->Currency.c_str());
    }
    if ( resultMediaType_.isFound() ) {
        Property* p = resultMediaType_.getProperty(context);
        if (p) p->setInt(tariffRec->MediaTypeId);
    }
    if ( resultCategory_.isFound() ) {
        Property* p = resultCategory_.getProperty(context);
        if (p) p->setInt(tariffRec->CategoryId);
    }
    if ( resultBillType_.isFound() ) {
        Property* p = resultBillType_.getProperty(context);
        if (p) p->setInt(tariffRec->billType);
    }
    if ( resultPrice_.isFound() ) {
        Property* p = resultPrice_.getProperty(context);
        if (p) p->setStr(tariffRec->getPrice().c_str());
    }
    setBillingStatus(context,"",true);
    return true;
}

}
}
}
