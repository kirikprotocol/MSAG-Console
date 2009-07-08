#include "ActionSessionDestroyService.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionSessionDestroyService::init( const SectionParams& params,
                                        PropertyObject propobj )
{
    smsc_log_debug( logger, "Action '%s': init", opname() );
    wait_.init(params,propobj);
}


bool ActionSessionDestroyService::run( ActionContext& context )
{
    smsc_log_debug( logger,"Action '%s': run", opname() );
    context.setDestroyService( wait_.isFound() ? wait_.getSeconds(context) : 0 );
    return true;
}


IParserHandler * ActionSessionDestroyService::StartXMLSubSection( const std::string&,
                                                                  const SectionParams&,
                                                                  const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool ActionSessionDestroyService::FinishXMLSubSection( const std::string& )
{
    return true;
}

}
}
}
