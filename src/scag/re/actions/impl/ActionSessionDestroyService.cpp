#include "ActionSessionDestroyService.h"

namespace {
const char* opname = "session:destroy_service";
}

namespace scag2 {
namespace re {
namespace actions {

void ActionSessionDestroyService::init( const SectionParams& params,
                                        PropertyObject propobj )
{
    smsc_log_debug( logger, "Action '%s': init", opname );
    haswait_ = wait_.init( params, propobj, opname, "wait", false, true );
}


bool ActionSessionDestroyService::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action '%s': run", opname );
    context.setDestroyService( haswait_ ? wait_.getTime(opname, context) : 0 );
    return true;
}


IParserHandler * ActionSessionDestroyService::StartXMLSubSection( const std::string&,
                                                                  const SectionParams&,
                                                                  const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname );
}


bool ActionSessionDestroyService::FinishXMLSubSection( const std::string& )
{
    return true;
}

}
}
}
