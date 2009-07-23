#include "BillAction.h"
#include "scag/bill/ewallet/Status.h"

namespace scag2 {
namespace re {
namespace actions {

BillAction::BillAction( bool transit ) : transit_(transit) {}

void BillAction::init( const SectionParams& params,
                       PropertyObject propertyObject )
{
    if ( ! transit_ ) {
        bool bExist;
        transIdFieldType_ = CheckParameter( params,
                                            propertyObject, 
                                            opname(), "id",
                                            true, true,
                                            transIdFieldName_,
                                            bExist );
    }

    /*
    std::string transitValue;
    bool hasTransit;
    FieldType transitType = CheckParameter( params,
                                            propertyObject,
                                            opname(),
                                            "transit",
                                            false, true,
                                            transitValue,
                                            hasTransit );
    if ( hasTransit ) {
        if ( transitType != ftUnknown ) {
            throw SCAGException( "Action '%s': transit field must be a bool constant", opname());
        }
        if ( transitValue == "yes" || transitValue == "true" || transitValue == "1" ) {
            transit_ = true;
        } else if ( transitValue == "no" || transitValue == "false" || transitValue == "0" ) {
            transit_ = false;
        } else {
            throw SCAGException( "Action '%s': transit field must be a bool constant", opname());
        }
    } else {
        transit_ = false;
    }
     */

    // --- output fields

    CheckParameter( params, propertyObject, 
                    opname(), "status",
                    false, false,
                    statusFieldName_,
                    hasStatus_ );

    CheckParameter( params, propertyObject,
                    opname(), "msg",
                    false, false,
                    msgFieldName_,
                    hasMessage_ );

}


IParserHandler * BillAction::StartXMLSubSection( const std::string&,
                                                 const SectionParams&,
                                                 const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool BillAction::FinishXMLSubSection( const std::string& )
{
    return true;
}


std::string BillAction::getTransId( ActionContext& context )
{
    std::string transId;
    if ( transit_ ) {
        smsc_log_error(logger,"logic error: transId requested for transit action '%s'", opname());
        return transId;
    }
    if ( transIdFieldType_ != ftUnknown ) {
        Property* property = context.getProperty( transIdFieldName_ );
        if ( ! property ) {
            setBillingStatus( context, "Invalid property for trans-id", false );
            smsc_log_error( logger, "Action '%s': invalid property %s for trans-id",
                            opname(), transIdFieldName_.c_str() );
            return transId;
        }
        transId = property->getStr().c_str();

    } else {

        transId = transIdFieldName_;
    }
    if ( transId.empty() ) {
        smsc_log_error( logger, "Action '%s': empty trans-id", opname() );
        setBillingStatus( context, "Empty transaction id", false );
    }
    return transId;
}


void BillAction::setBillingStatus( ActionContext& context,
                                   const char*    errmsg,
                                   bool           isok )
{
    setBillingStatus( context,errmsg,isok ?
                      bill::ewallet::Status::OK :
                      bill::ewallet::Status::UNKNOWN );
}


void BillAction::setBillingStatus( ActionContext& context,
                                   const char*    errmsg,
                                   int            statCode )
{
    if ( hasStatus_ ) {
        Property* propertyStatus = context.getProperty(statusFieldName_);
        if (!propertyStatus) 
        {
            smsc_log_debug( logger,"Action '%s' :: Invalid property %s for status",
                            opname(), statusFieldName_.c_str());
            return;
        }
        propertyStatus->setInt(statCode);
    }

    if (hasMessage_)
    {
        Property * propertyMsg = context.getProperty(msgFieldName_);

        if (!propertyMsg) 
        {
            smsc_log_debug( logger,"Action '%s' :: Invalid property %s for msg",
                            opname(), msgFieldName_.c_str());
            return;
        }
        propertyMsg->setStr(errmsg);
    }
}

}
}
}
