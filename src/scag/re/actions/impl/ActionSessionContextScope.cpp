#include "ActionSessionContextScope.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

ActionSessionContextScope::ActionSessionContextScope( ActionType tp ) :
type_(tp) {}


void ActionSessionContextScope::init( const SectionParams& params,
                                      PropertyObject propertyObject )
{
    FieldType ft;
    bool bExist;

    ft = CheckParameter( params, propertyObject, actionname(), "id",
                         true, (type_ == NEW ? false : true),
                         idfieldname_, bExist );

    CheckParameter( params, propertyObject, actionname(), "status",
                    false, true, statusfieldname_, hasstatus_ );

    // m_valueFieldType = CheckParameter(params, propertyObject, "set", "value", true, true, m_strValue, bExist);
    smsc_log_debug( logger, "Action '%s': init", actionname() );
}


bool ActionSessionContextScope::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action '%s': run", actionname() );
    Property * property = context.getProperty(idfieldname_);

    if (!property) 
    {
        smsc_log_warn(logger,"Action '%s': invalid property '%s'", actionname(), idfieldname_.c_str() );
        setstatus(false);
        return true;
    }

    switch (type_) {

    case NEW : {

        const int ctx = context.getSession().createContextScope();
        property->setInt( ctx );
        smsc_log_debug(logger,"Action '%s': property '%s' new scope_id='%d'", actionname(), idfieldname_.c_str(), ctx );
        setstatus(true);
        break;

    }

    case SET : {

        const int ctx = int(property->getInt());
        SessionPropertyScope* scope = context.getSession().getContextScope( ctx );
        if ( scope ) {
            context.setContextScope( ctx );
            smsc_log_debug(logger,"Action '%s': property '%s' set scope_id='%d' scope=%p", actionname(), idfieldname_.c_str(), ctx, scope );
        } else {
            smsc_log_warn(logger,"Action '%s': property '%s' cannot set scope_id='%d' - not found", actionname(), idfieldname_.c_str(), ctx );
        }
        setstatus( scope );
        break;
    }

    case DEL : {

        const int ctx = int(property->getInt());
        if ( context.getSession().deleteContextScope( ctx ) ) {
            const bool reset = ( context.getContextScope() == ctx );
            if ( reset ) context.setContextScope(0);
            smsc_log_debug(logger,"Action '%s': property '%s', context scope %d deleted%s",
                           actionname(), idfieldname_.c_str(), ctx,
                           reset ? " (reset also)" : "");
            setstatus(true);
        } else {
            smsc_log_warn(logger,"Action '%s': property '%s' cannot delete session scope %d - not found",
                          actionname(), idfieldname_.c_str(), ctx );
            setstatus(false);
        }
        break;

    }

    default:
        break;

    }
    return true;
}


IParserHandler * ActionSessionContextScope::StartXMLSubSection( const std::string& name,
                                                                const SectionParams& params,
                                                                const ActionFactory& factory )
{
    throw SCAGException( "Action '%s' cannot include child objects", actionname() );
}


bool ActionSessionContextScope::FinishXMLSubSection( const std::string& name )
{
    return true;
}

const char* ActionSessionContextScope::actionname() const
{
    switch ( type_ ) {
    case NEW : return "session:new_context";
    case SET : return "session:set_context";
    case DEL : return "session:del_context";
    default  : return "session:???_context";
    }
}


void ActionSessionContextScope::setstatus( bool st )
{
    if ( ! hasstatus_ ) return;
    Property * property = context.getProperty(statusfieldname_);
    if ( ! property ) {
        smsc_log_warn( logger,"Action '%s': property '%s' not found",
                       actionname(), statusfieldname_.c_str() );
        return;
    }
    property->setBool( st );
}

} // namespace actions
} // namespace re
} // namespace scag2
