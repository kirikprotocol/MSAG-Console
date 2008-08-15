#include "LongCallTestAction.h"

namespace scag2 {
namespace re {
namespace actions {

LongCallTestAction::LongCallTestAction()
{
}


LongCallTestAction::~LongCallTestAction()
{
}


void LongCallTestAction::init( const SectionParams&,
                               PropertyObject )
{
    smsc_log_debug( logger, "longcalltest: init" );
    // CheckParameter( params, propertyObject )
}


IParserHandler * LongCallTestAction::StartXMLSubSection( const std::string&,
                                                         const SectionParams&,
                                                         const ActionFactory& )
{
    throw SCAGException("Action 'longcalltest' cannot include child objects");
}


bool LongCallTestAction::FinishXMLSubSection( const std::string& )
{
    return true;
}


bool LongCallTestAction::RunBeforePostpone( ActionContext& )
{
    smsc_log_debug( logger, "longcalltest: run before postpone" );
    return true;
}


void LongCallTestAction::ContinueRunning( ActionContext& )
{
    smsc_log_debug( logger, "longcalltest: continue running" );
}

} // namespace actions
} // namespace re
} // namespace scag2
