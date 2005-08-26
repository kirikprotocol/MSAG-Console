#include "ActionClose.h"
#include "scag/re/CommandAdapter.h"

#include "scag/SAX2Print.hpp"

namespace scag { namespace re { namespace actions {


IParserHandler * ActionClose::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'session:close': cannot have a child object");
}

bool ActionClose::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (!params.Exists("commit")) throw SCAGException("Action 'session:close' missing 'commit' parameter");

    propertyName = params["commit"];



    const char * name;
    FieldType ft;
    ft = ActionContext::Separate(propertyName,name); 
    if (ft == ftUnknown) throw InvalidPropertyException("Action 'session:close': unrecognized variable prefix '",propertyName.c_str(),"' for 'commit' parameter");

    if (ft == ftField) 
    {
        AccessType at;
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) throw InvalidPropertyException("Action 'session:close': cannot read property '%s' - no access",propertyName.c_str());
    }

}

bool ActionClose::run(ActionContext& context)
{
    Property * p = context.getProperty(propertyName);

    bool Commit = false;
    if (!p) smsc_log_warn(logger,"Action 'session:close': invalid property '%s'",propertyName.c_str());
    else Commit = p->getBool();

    context.closeSession(Commit);

    return true;
}

ActionClose::~ActionClose()
{
}

}}}