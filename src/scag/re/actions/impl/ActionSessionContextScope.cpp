#include "ActionSessionContextScope.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

ActionSessionContextScope::ActionSessionContextScope( ActionType tp ) :
type_(tp) {
    switch ( type_ ) {
    case NEW : opname_ = "session:new_context"; break;
    case SET : opname_ = "session:set_context"; break;
    case DEL : opname_ = "session:del_context"; break;
    default  : opname_ = "session:???_context"; break;
    }
}


void ActionSessionContextScope::init( const SectionParams& params,
                                      PropertyObject propertyObject )
{
    FieldType ft;
    bool bExist;

    ft = CheckParameter( params, propertyObject, opname(), "id",
                         true, (type_ == NEW ? false : true),
                         idfieldname_, bExist );

    CheckParameter( params, propertyObject, opname(), "status",
                    false, true, statusfieldname_, hasstatus_ );

    // m_valueFieldType = CheckParameter(params, propertyObject, "set", "value", true, true, m_strValue, bExist);
    smsc_log_debug( logger, "Action '%s': init", opname() );
}


bool ActionSessionContextScope::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action '%s': run", opname() );
    Property * property = context.getProperty(idfieldname_);

    if (!property) 
    {
        smsc_log_warn(logger,"Action '%s': invalid property '%s'", opname(), idfieldname_.c_str() );
        setstatus(context,false);
        return true;
    }

    switch (type_) {

    case NEW : {

        const int ctx = context.getSession().createContextScope();
        property->setInt( ctx );
        context.setContextScope( ctx );
        smsc_log_debug(logger,"Action '%s': property '%s' new scope_id='%d'", opname(), idfieldname_.c_str(), ctx );
        setstatus(context,true);
        break;

    }

    case SET : {

        const int ctx = int(property->getInt());
        SessionPropertyScope* scope = context.getSession().getContextScope( ctx );
        if ( scope ) {
            context.setContextScope( ctx );
            smsc_log_debug(logger,"Action '%s': property '%s' set scope_id='%d' scope=%p", opname(), idfieldname_.c_str(), ctx, scope );
        } else {
            smsc_log_warn(logger,"Action '%s': property '%s' cannot set scope_id='%d' - not found", opname(), idfieldname_.c_str(), ctx );
        }
        setstatus( context,scope );
        break;
    }

    case DEL : {

        const int ctx = int(property->getInt());
        if ( context.getSession().deleteContextScope( ctx ) ) {
            const bool reset = ( context.getContextScope() == ctx );
            if ( reset ) context.setContextScope(0);
            smsc_log_debug(logger,"Action '%s': property '%s', context scope %d deleted%s",
                           opname(), idfieldname_.c_str(), ctx,
                           reset ? " (reset also)" : "");
            setstatus(context,true);
        } else {
            smsc_log_warn(logger,"Action '%s': property '%s' cannot delete session scope %d - not found",
                          opname(), idfieldname_.c_str(), ctx );
            setstatus(context,false);
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
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool ActionSessionContextScope::FinishXMLSubSection( const std::string& name )
{
    return true;
}


void ActionSessionContextScope::setstatus( ActionContext& context, bool st )
{
    if ( ! hasstatus_ ) return;
    Property * property = context.getProperty(statusfieldname_);
    if ( ! property ) {
        smsc_log_warn( logger,"Action '%s': property '%s' not found",
                       opname(), statusfieldname_.c_str() );
        return;
    }
    property->setBool( st );
}

} // namespace actions
} // namespace re
} // namespace scag2
