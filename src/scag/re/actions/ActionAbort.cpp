#include "ActionAbort.h"
#include "scag/re/CommandAdapter.h"

#include "scag/SAX2Print.hpp"

namespace scag { namespace re { namespace actions {


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
    //if (!params.GetCount()) throw SCAGException("Action 'session:close' must have no parameters");
}

bool ActionAbort::run(ActionContext& context)
{
    context.abortSession();
    return true;
}

ActionAbort::~ActionAbort()
{
}

}}}