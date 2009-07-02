#include "BillActionMkid.h"

namespace scag2 {
namespace re {
namespace actions {

void BillActionMkid::init( const SectionParams& params,
                           PropertyObject propertyObject )
{
    bool bExist;
    idtype_ = CheckParameter( params,
                              propertyObject, 
                              opname(), "externalId",
                              true, false,
                              idname_,
                              bExist );
}

IParserHandler * BillActionMkid::StartXMLSubSection( const std::string&,
                                                     const SectionParams&,
                                                     const ActionFactory& )
{
    throw SCAGException( "Action '%s' cannot include child objects", opname() );
}


bool BillActionMkid::FinishXMLSubSection( const std::string& )
{
    return true;
}


bool BillActionMkid::run( ActionContext& context )
{
    Property * property = context.getProperty(idname_);
    if (!property) {
        throw SCAGException("Action '%s': Invalid property for externalId", opname());
    }
    const unsigned long long msec = static_cast<unsigned long long>(util::currentTimeMillis());
    CommandProperty& prop = context.getCommandProperty();
    char abuf[50];
    prop.abonentAddr.getText(abuf,sizeof(abuf));
    char buf[100];
    snprintf(buf,sizeof(buf),"msag-%llu-%u-%s", msec, prop.serviceId, abuf );
    property->setStr(buf);
    return true;
}

}
}
}
