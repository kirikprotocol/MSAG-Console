#include "SessionAdapter2.h"

namespace scag2 { namespace re { namespace session
{

AccessType SessionAdapter::CheckAccess(int handlerType, const std::string& name)
{
    if(!strcmp(name.c_str(), "status")) return atRead;
    return atNoAccess;
}

SessionAdapter::~SessionAdapter()
{
    if(statusProp) delete statusProp;
}

Property* SessionAdapter::getProperty(const std::string& name)
{
    if(!strcmp(name.c_str(), "status"))
    {
        if(!statusProp)
            statusProp = new AdapterProperty(name, this, false );
        return statusProp;
    }
    return NULL;
}

void SessionAdapter::changed(AdapterProperty& property)
{
}

}}}
