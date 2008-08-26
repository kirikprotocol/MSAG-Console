#include "ActionAbort2.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {


IParserHandler * ActionAbort::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'session:close': cannot have a child object");
}

bool ActionAbort::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionAbort::init(const SectionParams& params,PropertyObject propertyObject)
{

    //if (params.GetCount()) throw SCAGException("Action 'session:abort' must have no parameters");
}

bool ActionAbort::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'abort'...");
    context.abortSession();
    return true;
}

ActionAbort::~ActionAbort()
{

}

}}}
