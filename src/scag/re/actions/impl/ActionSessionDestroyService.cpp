#include "ActionSessionDestroyService.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionSessionDestroyService::init( const SectionParams&,
                                        PropertyObject )
{
    smsc_log_debug( logger, "Action 'session:destroy_service': init" );
}


bool ActionSessionDestroyService::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action 'session:destroy_service': run" );
    context.setDestroyService();
    return true;
}


IParserHandler * ActionSessionDestroyService::StartXMLSubSection( const std::string&,
                                                                  const SectionParams&,
                                                                  const ActionFactory& )
{
    throw SCAGException( "Action 'session:destroy_service' cannot include child objects" );
}


bool ActionSessionDestroyService::FinishXMLSubSection( const std::string& )
{
    return true;
}

}
}
}
