#include "ActionClose.h"
//#include <string>

namespace scag { namespace re { namespace actions {

IParserHandler * ActionClose::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw Exception("Action 'session:close': cannot have a child object");
}

bool ActionClose::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionClose::init(const SectionParams& params)
{
    if (!params.Exists("commit")) throw Exception("Action 'session:close' missing 'commit' parameter");

    propertyName = params["commit"];
}

bool ActionClose::run(ActionContext& context)
{
    
    return true;
}

ActionClose::~ActionClose()
{
}

}}}