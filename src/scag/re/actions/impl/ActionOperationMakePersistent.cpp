#include "ActionOperationMakePersistent.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionOperationMakePersistent::init( const SectionParams&,
                                          PropertyObject )
{
    smsc_log_debug( logger, "Action 'operation:make_persistent': init" );
}


bool ActionOperationMakePersistent::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action 'operation:make_persistent': run" );
    if ( context.getSession().getCurrentOperation() )
        context.getSession().getCurrentOperation()
            ->setFlag( OperationFlags::PERSISTENT );
    return true;
}


IParserHandler * ActionOperationMakePersistent::StartXMLSubSection( const std::string&,
                                                                    const SectionParams&,
                                                                    const ActionFactory& )
{
    throw SCAGException( "Action 'operation:make_persistent' cannot include child objects" );
}


bool ActionOperationMakePersistent::FinishXMLSubSection( const std::string& )
{
    return true;
}

}
}
}
