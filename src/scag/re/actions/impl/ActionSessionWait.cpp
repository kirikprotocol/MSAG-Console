#include <time.h>
#include <cstdlib>
#include "ActionSessionWait.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionSessionWait::init( const SectionParams& params,
                              PropertyObject propertyObject )
{
    // --- input fields
    wait_.init(params, propertyObject);
}


IParserHandler * ActionSessionWait::StartXMLSubSection( const std::string&,
                                                        const SectionParams&,
                                                        const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool ActionSessionWait::FinishXMLSubSection( const std::string& )
{
    return true;
}


bool ActionSessionWait::run( ActionContext& context )
{
    const unsigned tmo = wait_.getSeconds( context );
    context.getSession().waitAtLeast( tmo );
    smsc_log_debug( logger, "Action 'session:wait': tmo=%u", tmo );
    return true;
}

}
}
}
