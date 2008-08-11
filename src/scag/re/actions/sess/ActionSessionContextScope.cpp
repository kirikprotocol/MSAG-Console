#include "ActionSessionContextScope.h"

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

    // m_valueFieldType = CheckParameter(params, propertyObject, "set", "value", true, true, m_strValue, bExist);
    smsc_log_debug( logger, "Action '%s':: init", actionname() );
}


bool ActionSessionContextScope::run( ActionContext& context )
{
    smsc_log_debug( logger,"Run Action '%s'", actionname() );
    Property * property = context.getProperty(idfieldname_);

    if (!property) 
    {
        smsc_log_warn(logger,"Action '%s':: invalid property '%s'", actionname(), idfieldname_.c_str() );
        return true;
    }

    switch (type_) {

    case NEW : {

        const int ctx = context.getSession().createContextScope();
        property->setInt( ctx );
        smsc_log_debug(logger,"Action '%s': property '%s' set to '%d'", actionname(), idfieldname_.c_str(), ctx );
        break;

    }

    case SET : {

        const int ctx = int(property->getInt());
        SessionPropertyScope* scope = context.getSession().getContextScope( ctx );
        if ( scope ) {
            context.setContextScope( ctx );
            smsc_log_debug(logger,"Action '%s': property '%s' set session context scope to '%d'", actionname(), idfieldname_.c_str(), ctx );
        } else {
            smsc_log_warn(logger,"Action '%s': property '%s' cannot set session context scope to '%d' - not found", actionname(), idfieldname_.c_str(), ctx );
        }
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
        } else
            smsc_log_warn(logger,"Action '%s': property '%s' cannot delete session scope %d - not found",
                          actionname(), idfieldname_.c_str(), ctx );
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

} // namespace actions
} // namespace re
} // namespace scag2
