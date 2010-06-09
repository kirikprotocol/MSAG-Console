#include "ActionDel.h"

namespace scag2 {
namespace re {
namespace actions {


void ActionDel::init(const SectionParams& params,PropertyObject propertyObject)
{
    varname_.init(params,propertyObject);
    if ( varname_.getType() == ftConst ||
         varname_.getType() == ftUnknown ) {
        throw SCAGException("Action '%s': constants/unknowns cannot be deleted",opname());
    }
    smsc_log_debug(logger,"Action '%s': init",opname());
}


bool ActionDel::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action '%s', prop=%s",opname(), varname_.getStringValue() );
    context.delProperty( varname_.getStringValue() );
    return true;
}


IParserHandler * ActionDel::StartXMLSubSection( const std::string& name,
                                                const SectionParams& params,
                                                const ActionFactory& factory )
{
    throw SCAGException("Action 'del' cannot include child objects");
}


bool ActionDel::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}
}
}
