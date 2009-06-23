#include "BillActionInfo.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
#include "scag/sessions/base/Session2.h"
// #include "scag/re/CommandAdapter.h"
// #include "scag/util/lltostr.h"

namespace scag2 {
namespace re {
namespace actions {

const char* BillActionInfo::m_name[BillActionInfo::fields_count] = 
{
        "operator_id", 
        "provider_id",
        "service_id",
        "abonent",
        "category",
        "content-type",
        "result_number",
        "price",
        "currency",
        "bill_type",
        "category-str",
        "content-type-str",
        "keywords"
};

void BillActionInfo::init( const SectionParams& params,
                           PropertyObject propertyObject )
{
    BillAction::init( params, propertyObject );

    for ( int i = 0; i < fields_count; i++ )
        CheckParameter( params, propertyObject,
                        opname(), m_name[i],
                        false, false,
                        fieldName_[i], hasField_[i] );

    smsc_log_debug(logger,"Action '%s' init...", opname() );
}


bool BillActionInfo::run( ActionContext& context )
{
    smsc_log_debug( logger,"Run Action '%s'...", opname() );

    std::string transId = getTransId( context );
    if ( transId.empty() ) {
        // smsc_log_error( logger, "Action '%s' cannot get transaction name", opname() );
        // setBillingStatus( context, "cannot get transaction name", false );
        return true;
    }

    ExternalTransaction* etrans = 
        context.getSession().getTransaction( transId.c_str() );
    if ( ! etrans ) {
        smsc_log_error( logger, "Action '%s' transaction '%s' not found",
                        opname(), transId.c_str() );
        setBillingStatus( context, "transaction not found", false );
        return true;
    }

    ExternalBillingTransaction* trans = etrans->castToBilling();
    if ( ! trans ) {
        smsc_log_error( logger, "Action '%s' transaction '%s' is not a billing one",
                        opname(), transId.c_str() );
        setBillingStatus( context, "not a billing transaction", false );
        return true;
    }
    
    bill::BillingInfoStruct bis;
    bill::TariffRec tr;

    try {
        trans->info( bis, tr );
    }
    catch (SCAGException& e)
    {
        smsc_log_error( logger, "Action '%s': BM->info failed trans=%s bill_id=%d. Error: %s",
                        opname(), transId.c_str(), trans->billId(), e.what());
        setBillingStatus(context, e.what(), false);
        return true;
    }

    std::string s;
    if ( logger->isDebugEnabled() ) {
        char buf[20];
        snprintf( buf, sizeof(buf), "%u", trans->billId() );
        s = std::string("Action 'bill:info' bill_id=") + std::string(buf);
        s.reserve( s.size() + 200 );
    }
    for ( int i = 0; i < fields_count; i++ )
    {
        if ( !hasField_[i] ) continue;

        Property *p = context.getProperty( fieldName_[i] );
        if (!p)
        {
            const std::string what = "Action 'bill:info' :: Invalid property " +
                fieldName_[i] + " for " +  m_name[i];
            smsc_log_error(logger, what.c_str());
            setBillingStatus(context, what.c_str(), false);
            return true;
        }
        switch (i)
        {
        case 0: p->setInt(bis.operatorId); break;
        case 1: p->setInt(bis.providerId); break;
        case 2: p->setInt(bis.serviceId); break;
        case 3: p->setStr(bis.AbonentNumber.c_str()); break;
        case 4: p->setInt(tr.CategoryId); break;
        case 5: p->setInt(tr.MediaTypeId); break;
        case 6: p->setStr(tr.ServiceNumber.c_str()); break;
        case 7: {
            //char bufp[20];
            //snprintf(bufp, sizeof(bufp), "%.3lf", tr.Price);
            p->setStr(tr.getPrice().c_str());
            break;
        }
        case 8: p->setStr(tr.Currency.c_str()); break;
        case 9: p->setInt(tr.billType); break;
        case 10: p->setStr(bis.category.c_str()); break;
        case 11: p->setStr(bis.mediaType.c_str()); break;
        //TODO: set real keywords from action context
        case 12:  {
            const std::string* keywords = trans->getKeywords();
            p->setStr(keywords ? keywords->c_str() : "" );
          break;
        }
        }
        if ( logger->isDebugEnabled() ) {
            s += ", ";
            s += m_name[i];
            s.push_back('=');
            s.append(p->getStr().c_str());
        }
    }
    smsc_log_debug( logger, s );
    setBillingStatus(context, "", true);
    return true;
}

}}}

